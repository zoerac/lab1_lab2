#include "./src/lib.h"
#include "./src/kv.h"
#include "./src/wh.h"
#include "../indexInterface.h"
#include "omp.h"


static struct kv* kv_dup_in_count(struct kv* kv, void* priv) {
  int64_t current_usage = *((int64_t*)priv);
  *((int64_t*)priv) = current_usage + kv_size(kv);
  return kv_dup(kv);
}

static struct kv* kv_dup_out_count(struct kv* kv, struct kv* out) {
  return kv_dup2(kv, out);
}

static void kv_free_count(struct kv* kv, void* priv) {
  int64_t current_usage = *((int64_t*)priv);
  *((int64_t*)priv) = current_usage - kv_size(kv);
  free(kv);
}

template <class KeyType>
static void wh_convert2String(const KeyType& key, std::string& str){
  KeyType endian_key;
  if constexpr (std::is_same<KeyType, uint32_t>::value){
    endian_key = __builtin_bswap32(key);
  }
  else if constexpr (std::is_same<KeyType, uint64_t>::value){
    endian_key = __builtin_bswap64(key);
  }
  else{
    assert(0 == 1);
  }
  str = std::string(reinterpret_cast<const char*>(&endian_key), sizeof(KeyType));
}

template<class KEY_TYPE, class PAYLOAD_TYPE>
class WormholeU64Interface : public indexInterface<KEY_TYPE, PAYLOAD_TYPE> {
public:

    WormholeU64Interface() {
        //wh = wormhole_u64_create(&mm_nnn); 
    }

    void init(Param *param = nullptr) {
        // this->thread_num = param->worker_num;
        // for (int i = 0; i < thread_num; i++) {
            // ref[i].instance = wormhole_ref(wh);
        // }
    }

    void bulk_load(std::pair <KEY_TYPE, PAYLOAD_TYPE> *key_value, size_t num, Param *param = nullptr);

    bool get(KEY_TYPE key, PAYLOAD_TYPE &val, Param *param = nullptr);

    bool put(KEY_TYPE key, PAYLOAD_TYPE value, Param *param = nullptr);

    bool update(KEY_TYPE key, PAYLOAD_TYPE value, Param *param = nullptr);

    bool remove(KEY_TYPE key, Param *param = nullptr);

    size_t scan(KEY_TYPE key_low_bound, size_t key_num, std::pair <KEY_TYPE, PAYLOAD_TYPE> *result,
                Param *param = nullptr) {
        uint32_t thread_id = param->thread_id;
        auto iter = wormhole_iter_create(refs[thread_id].instance);
        std::string lkey;
        wh_convert2String(key_low_bound, lkey);
        kv_refill(in[thread_id], lkey.c_str(), lkey.length(), NULL, 0);
        kref kref = kv_kref(in[thread_id]);
        whsafe_iter_seek(iter, &kref);

        uint64_t cnt = 0;
        while (cnt < key_num && wormhole_iter_next(iter, out[thread_id])){
          result[cnt].second = *(uint64_t*)kv_vptr(out[thread_id]);
          ++cnt;
        }

        whsafe_iter_destroy(iter);
        return cnt;
    }

    ~WormholeU64Interface() {
        if (index){
          for (size_t i = 0; i < num_threads_; i++) {
            free(in[i]);
            free(out[i]);
            wormhole_unref(refs[i].instance);
          }
          delete[] refs;
          delete[] in;
          delete[] out;
          wormhole_destroy(index);
        }
    }

    long long memory_consumption() { return usage_; }

private:
    struct alignas(CACHELINE_SIZE) wormref_align {
        struct wormref *instance;
    };
    struct wormhole* index = NULL;
    struct wormref_align* refs = NULL;
    int64_t usage_ = 0;
    size_t num_threads_;
    struct kv ** in = NULL;
    struct kv ** out = NULL;
};

template<class KEY_TYPE, class PAYLOAD_TYPE>
void WormholeU64Interface<KEY_TYPE, PAYLOAD_TYPE>::bulk_load(std::pair <KEY_TYPE, PAYLOAD_TYPE> *key_value, size_t num,Param *param) {
    std::vector<std::string> keys;
    keys.reserve(num);
    //for (const std::pair <KEY_TYPE, PAYLOAD_TYPE>& kv : key_value) {
    for (size_t i = 0; i < num; ++i){
      std::string key;
      wh_convert2String(key_value[i].first, key);
      keys.push_back(key);
    }

    kvmap_mm allocator = (kvmap_mm){kv_dup_in_count, kv_dup_out_count,
                                    kv_free_count, (void*)&usage_};
    num_threads_ = param->worker_num;
    refs = new struct wormref_align[num_threads_];
    in = new struct kv*[num_threads_];
    out = new struct kv*[num_threads_];
    for (size_t i = 0; i < num_threads_; ++ i){
      in[i] = static_cast<struct kv*>(malloc(sizeof(struct kv) + 1024 + sizeof(uint64_t)));
      out[i] = static_cast<struct kv*>(malloc(sizeof(struct kv) + 1024 + sizeof(uint64_t)));
    }
    index = wormhole_create(&allocator);
    struct wormref * ref = wormhole_ref(index);
    for (size_t i = 0; i < num; i++) {
      kv_refill(in[0], keys[i].c_str(), keys[i].length(), reinterpret_cast<const char* const>(&key_value[i].second), sizeof(uint64_t));
      wormhole_put(ref, in[0]);
    }
    wormhole_unref(ref);
    for (size_t i = 0; i < num_threads_; ++ i){
      refs[i].instance = whsafe_ref(index);
    }
}

template<class KEY_TYPE, class PAYLOAD_TYPE>
bool WormholeU64Interface<KEY_TYPE, PAYLOAD_TYPE>::get(KEY_TYPE key, PAYLOAD_TYPE &val, Param *param) {
    std::string _key;
    uint32_t thread_id = param->thread_id;
    wh_convert2String(key, _key);
    kv_refill(in[thread_id], _key.c_str(), _key.length(), NULL, 0);
    kref kref = kv_kref(in[thread_id]);
    if (whsafe_get(refs[thread_id].instance, &kref, out[thread_id])){
      val = *(uint64_t*)kv_vptr(out[thread_id]);
      return true;
    }
    return false;
}

template<class KEY_TYPE, class PAYLOAD_TYPE>
bool WormholeU64Interface<KEY_TYPE, PAYLOAD_TYPE>::put(KEY_TYPE key, PAYLOAD_TYPE value, Param *param) {
    std::string _key; 
    uint32_t thread_id = param->thread_id;
    wh_convert2String(key, _key);
    kv_refill(in[thread_id], _key.c_str(), _key.length(), reinterpret_cast<const char* const>(&value), sizeof(uint64_t));
    return whsafe_put(refs[thread_id].instance, in[thread_id]);
}

template<class KEY_TYPE, class PAYLOAD_TYPE>
bool WormholeU64Interface<KEY_TYPE, PAYLOAD_TYPE>::update(KEY_TYPE key, PAYLOAD_TYPE value, Param *param) {
    //struct wormref * const ref = wormhole_ref(index);
    //auto ret = wormhole_u64_set(ref, key, (void *)value);
    //wormhole_unref(ref);
    //return ret;
    return 0;
}

template<class KEY_TYPE, class PAYLOAD_TYPE>
bool WormholeU64Interface<KEY_TYPE, PAYLOAD_TYPE>::remove(KEY_TYPE key, Param *param) {
    //struct wormref * const ref = wormhole_ref(index);
    //auto ret = wormhole_u64_del(ref, key);
    //wormhole_unref(ref);
    //return ret;
    return 0;
}
