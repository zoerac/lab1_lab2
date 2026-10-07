#include"./PGM-index/include/pgm_index_dynamic.hpp"
#include"../indexInterface.h"

template<class KEY_TYPE, class PAYLOAD_TYPE>
class pgmTliInterface : public indexInterface<KEY_TYPE, PAYLOAD_TYPE> {
public:
  void init(Param *param = nullptr) {}

  void bulk_load(std::pair <KEY_TYPE, PAYLOAD_TYPE> *key_value, size_t num, Param *param = nullptr);

  bool get(KEY_TYPE key, PAYLOAD_TYPE &val, Param *param = nullptr);

  bool put(KEY_TYPE key, PAYLOAD_TYPE value, Param *param = nullptr);

  bool update(KEY_TYPE key, PAYLOAD_TYPE value, Param *param = nullptr);

  bool remove(KEY_TYPE key, Param *param = nullptr);

  size_t scan(KEY_TYPE key_low_bound, size_t key_num, std::pair<KEY_TYPE, PAYLOAD_TYPE> *result,
              Param *param = nullptr);

  long long memory_consumption() { return index->size_in_bytes(); }

  void print_stats(double &avg_height){
    index->print_stats(avg_height);
  }

private:
  //DynamicPGMIndex<KEY_TYPE, PAYLOAD_TYPE> *index;
  pgmtli::DynamicPGMIndex<KEY_TYPE, PAYLOAD_TYPE, pgmtli::PGMIndex<KEY_TYPE, 64, 16> > *index;
};

template<class KEY_TYPE, class PAYLOAD_TYPE>
void pgmTliInterface<KEY_TYPE, PAYLOAD_TYPE>::bulk_load(std::pair <KEY_TYPE, PAYLOAD_TYPE> *key_value, size_t num,
                                                     Param *param) {
  index = new pgmtli::DynamicPGMIndex<KEY_TYPE, PAYLOAD_TYPE>(key_value, key_value + num);
}

template<class KEY_TYPE, class PAYLOAD_TYPE>
bool pgmTliInterface<KEY_TYPE, PAYLOAD_TYPE>::get(KEY_TYPE key, PAYLOAD_TYPE &val, Param *param) {
  auto iter = index->find(key);
  if (iter  != index->end()) {
    val = iter->value();
    return true;
  }
  return false;
}

template<class KEY_TYPE, class PAYLOAD_TYPE>
bool pgmTliInterface<KEY_TYPE, PAYLOAD_TYPE>::put(KEY_TYPE key, PAYLOAD_TYPE value, Param *param) {
  //index->insert_or_assign(key, value);
  index->insert(key, value);
  return true;
}

template<class KEY_TYPE, class PAYLOAD_TYPE>
bool pgmTliInterface<KEY_TYPE, PAYLOAD_TYPE>::update(KEY_TYPE key, PAYLOAD_TYPE value, Param *param) {
  //index->insert_or_assign(key, value);
  index->insert(key, value);
  return true;
}

template<class KEY_TYPE, class PAYLOAD_TYPE>
bool pgmTliInterface<KEY_TYPE, PAYLOAD_TYPE>::remove(KEY_TYPE key, Param *param) {
  index->erase(key);
  return true;
}

template<class KEY_TYPE, class PAYLOAD_TYPE>
size_t pgmTliInterface<KEY_TYPE, PAYLOAD_TYPE>::scan(KEY_TYPE key_low_bound, size_t key_num,
                                                  std::pair<KEY_TYPE, PAYLOAD_TYPE> *result,
                                                  Param *param) {
  auto iter = index->lower_bound(key_low_bound);
  int scan_size = 0;
  PAYLOAD_TYPE accumulator = 0;
  for (scan_size = 0; scan_size < key_num && iter != index->end(); scan_size++) {
    result[scan_size] = {iter->key(),iter->value()};
    ++iter;
  }
  return scan_size;
}