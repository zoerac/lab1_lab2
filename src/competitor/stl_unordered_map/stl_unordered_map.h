#include <unordered_map>
#include"../indexInterface.h"

template<class KEY_TYPE, class PAYLOAD_TYPE>
class STLUnorderedMapInterface : public indexInterface<KEY_TYPE, PAYLOAD_TYPE> {
public:
  void init(Param *param = nullptr) {}

  void bulk_load(std::pair <KEY_TYPE, PAYLOAD_TYPE> *key_value, size_t num, Param *param = nullptr){
    for (size_t i = 0; i < num; ++i){
        idx[key_value[i].first] = key_value[i].second;
    }
  }

  bool get(KEY_TYPE key, PAYLOAD_TYPE &val, Param *param = nullptr){
    auto iter = idx.find(key);
    if (iter != idx.end()){ 
        val = iter->second;
        return true;
    }
    return false;
  }

  

  bool put(KEY_TYPE key, PAYLOAD_TYPE value, Param *param = nullptr){
    auto res = idx.insert(std::make_pair(key, value));
    return res.second;
  }

  bool update(KEY_TYPE key, PAYLOAD_TYPE value, Param *param = nullptr){
    auto iter = idx.find(key);
    if (iter != idx.end()){
      iter->second = value;
      return true;
    }
    else
      return false;
  }

  bool remove(KEY_TYPE key, Param *param = nullptr){
    auto iter = idx.find(key);
    if (iter != idx.end()){
        idx.erase(iter);
        return true;
    }
    else{
        return false;
    }
  }

  size_t scan(KEY_TYPE key_low_bound, size_t key_num, std::pair<KEY_TYPE, PAYLOAD_TYPE> *result,
              Param *param = nullptr) {
                
    return 0;
  }

  long long memory_consumption() { return 0; }

  void print_stats(double &avg_height){}

private:
  std::unordered_map<KEY_TYPE, PAYLOAD_TYPE> idx;
};

