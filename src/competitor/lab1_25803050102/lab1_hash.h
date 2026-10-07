#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include "../indexInterface.h"

template<class KEY_TYPE, class PAYLOAD_TYPE>
class Lab1HashInterface : public indexInterface<KEY_TYPE, PAYLOAD_TYPE> {
  static_assert(std::is_same<KEY_TYPE, uint64_t>::value,
                "Lab1 requires uint64_t keys");
  static_assert(std::is_same<PAYLOAD_TYPE, uint64_t>::value,
                "Lab1 memory accounting requires uint64_t payloads");
  using Entry = std::pair<KEY_TYPE, PAYLOAD_TYPE>;
  enum : uint8_t { Empty, Occupied, Deleted };

  std::vector<Entry> entries_;
  std::vector<uint8_t> states_;
  size_t size_ = 0;
  size_t used_ = 0; // Occupied plus tombstones.
  size_t peak_bytes_ = sizeof(Lab1HashInterface);

  static size_t hash(KEY_TYPE key) {
    // SplitMix64 finalizer: mix all key bits before masking the bucket number.
    key ^= key >> 30;
    key *= UINT64_C(0xbf58476d1ce4e5b9);
    key ^= key >> 27;
    key *= UINT64_C(0x94d049bb133111eb);
    return static_cast<size_t>(key ^ (key >> 31));
  }

  static size_t limit(size_t capacity) { return capacity - capacity / 5; }

  static size_t doubled(size_t capacity) {
    if (capacity > std::numeric_limits<size_t>::max() / 2)
      throw std::length_error("Lab1 hash capacity overflow");
    return capacity * 2;
  }

  size_t allocated_bytes() const {
    return entries_.capacity() * sizeof(Entry) + states_.capacity();
  }

  // Returns either the matching slot or the first reusable slot. There is
  // always an Empty slot: rehash before non-empty occupancy exceeds ~80%.
  size_t locate(KEY_TYPE key) const {
    size_t slot = hash(key) & (states_.size() - 1);
    size_t deleted = states_.size();
    while (states_[slot] != Empty) {
      if (states_[slot] == Occupied && entries_[slot].first == key)
        return slot;
      if (states_[slot] == Deleted && deleted == states_.size())
        deleted = slot;
      slot = (slot + 1) & (states_.size() - 1);
    }
    return deleted == states_.size() ? slot : deleted;
  }

  void rehash(size_t capacity) {
    std::vector<Entry> entries(capacity);
    peak_bytes_ = std::max(peak_bytes_, sizeof(*this) + allocated_bytes()
                          + entries.capacity() * sizeof(Entry));
    std::vector<uint8_t> states(capacity, Empty);
    // Old and new arrays coexist until the local vectors are destroyed.
    peak_bytes_ = std::max(peak_bytes_, sizeof(*this) + allocated_bytes()
                          + entries.capacity() * sizeof(Entry) + states.capacity());
    for (size_t i = 0; i < states_.size(); ++i) {
      if (states_[i] != Occupied) continue;
      size_t slot = hash(entries_[i].first) & (capacity - 1);
      while (states[slot] == Occupied) slot = (slot + 1) & (capacity - 1);
      entries[slot] = entries_[i];
      states[slot] = Occupied;
    }
    entries_.swap(entries);
    states_.swap(states);
    used_ = size_;
  }

public:
  Lab1HashInterface() = default;
  Lab1HashInterface(const Lab1HashInterface&) = delete;
  Lab1HashInterface& operator=(const Lab1HashInterface&) = delete;

  void init(Param* = nullptr) override {
    std::vector<Entry>().swap(entries_);
    std::vector<uint8_t>().swap(states_);
    size_ = used_ = 0;
    // Preserve lifetime peak even when the index is reset or rebuilt.
  }

  void bulk_load(Entry* values, size_t num, Param* = nullptr) override {
    init();
    if (num == 0) return;
    size_t capacity = 16;
    while (num > limit(capacity)) capacity = doubled(capacity);
    rehash(capacity);
    for (size_t i = 0; i < num; ++i) {
      const size_t slot = locate(values[i].first);
      if (states_[slot] != Occupied) {
        states_[slot] = Occupied;
        ++size_;
        ++used_;
      }
      // Like the supplied STL bulk loader, the last duplicate value wins.
      entries_[slot] = values[i];
    }
  }

  bool get(KEY_TYPE key, PAYLOAD_TYPE& value, Param* = nullptr) override {
    if (states_.empty()) return false;
    const size_t slot = locate(key);
    if (states_[slot] != Occupied) return false;
    value = entries_[slot].second;
    return true;
  }

  bool put(KEY_TYPE key, PAYLOAD_TYPE value, Param* = nullptr) override {
    if (states_.empty()) rehash(16);
    size_t slot = locate(key);
    if (states_[slot] == Occupied) return false;
    if (states_[slot] == Empty && used_ >= limit(states_.size())) {
      // ponytail: scalar linear probing; add grouped probes only if measured
      // clustering is a bottleneck. Rebuild in place when tombstones dominate.
      rehash(size_ < limit(states_.size()) ? states_.size() : doubled(states_.size()));
      slot = locate(key);
    }
    entries_[slot] = {key, value};
    if (states_[slot] == Empty) ++used_;
    states_[slot] = Occupied;
    ++size_;
    return true;
  }

  bool update(KEY_TYPE key, PAYLOAD_TYPE value, Param* = nullptr) override {
    if (states_.empty()) return false;
    const size_t slot = locate(key);
    if (states_[slot] != Occupied) return false;
    entries_[slot].second = value;
    return true;
  }

  bool remove(KEY_TYPE key, Param* = nullptr) override {
    if (states_.empty()) return false;
    const size_t slot = locate(key);
    if (states_[slot] != Occupied) return false;
    states_[slot] = Deleted;
    --size_;
    return true;
  }

  size_t scan(KEY_TYPE, size_t, Entry*, Param* = nullptr) override { return 0; }

  long long memory_consumption() override {
    return static_cast<long long>(peak_bytes_);
  }
};
