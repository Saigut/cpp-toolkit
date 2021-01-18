#ifndef CPP_TOOLKIT_MOD_HASH_TABLE_H
#define CPP_TOOLKIT_MOD_HASH_TABLE_H

#include <stdint.h>
#include <memory>
#include <map>
#include <vector>

#include <mod_common/timeout.h>

template <typename K, typename V>
class cpt_hash_table {
public:
    cpt_hash_table() : table() {}
    bool insert(const K& key, const V &val);
    bool del(const K& key);
    bool find(const K& key, V& val);
    ~cpt_hash_table();
private:
    std::map<K, V> table;
    struct timeouts* m_to;
};

//template<typename K, typename V>
//bool cpt_hash_table<K, V>::init() {
//    if (!this->table) {
//        this->table = std::make_shared<std::map<K, V>>();
//    }
//    int err;
//    this->m_to = timeouts_open(TIMEOUT_mHZ, &err);
//    expect_ret_val(this->m_to, false);
//    return true;
//}

template<typename K, typename V>
bool cpt_hash_table<K, V>::insert(const K &key, const V &val) {
//    auto to = (struct timeout*)malloc(sizeof(struct timeout));
//    expect_ret_val(to, false);
//    timeout_init(to, 0);
//    K* p_key = (K*)malloc(sizeof(K));
//    *p_key = key;
//    to->callback.arg = p_key;
    this->table.insert({key, val});

//    timeouts_add(m_to, to, 10000);
    return true;
}

template<typename K, typename V>
bool cpt_hash_table<K, V>::del(const K &key) {
    this->table.erase(key);
    return true;
}

template<typename K, typename V>
bool cpt_hash_table<K, V>::find(const K &key, V &val) {
    std::map<K, V>::iterator itr;
    itr = this->table.find(key);
    if (itr != this->table.end()) {
        val = itr->second;
        return true;
    } else {
        return false;
    }
}

template<typename K, typename V>
cpt_hash_table<K, V>::~cpt_hash_table() {
    if (this->m_to) {
        timeouts_close(this->m_to);
    }
}

#endif //CPP_TOOLKIT_MOD_HASH_TABLE_H
