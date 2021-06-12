#ifndef CPP_TOOLKIT_MOD_HASH_TABLE_H
#define CPP_TOOLKIT_MOD_HASH_TABLE_H

#include <stdint.h>
#include <memory>
#include <map>
#include <vector>

#include <mod_common/timeout.h>
#include <mod_common/expect.h>
#include <ctime>

template <typename K, typename V>
class cpt_hash_table {
public:
    struct value_wrap_t {
        struct timeout* to;
        V value;
    };
    cpt_hash_table() : table() {}
    bool init();
    bool insert(const K& key, const V &val);
    bool del(const K& key);
    bool find(const K& key, V& val);
    void timeout_update();
    ~cpt_hash_table();
private:
    std::map<K, value_wrap_t> table;
    struct timeouts* m_to;
};

template<typename K, typename V>
bool cpt_hash_table<K, V>::init() {
//    if (!this->table) {
//        this->table = std::make_shared<std::map<K, V>>();
//    }
    int err;
    // create timing wheel
    this->m_to = timeouts_open(TIMEOUT_mHZ, &err);
    expect_ret_val(nullptr != this->m_to, false);
    return true;
}

template<typename K, typename V>
bool cpt_hash_table<K, V>::insert(const K &key, const V &val) {
    struct timeout* to = nullptr;
    K* p_key = nullptr;
    value_wrap_t value_wrap;

    to = (struct timeout*)malloc(sizeof(struct timeout));
    expect_goto(to, fail_return);
    timeout_init(to, 0);

    p_key = (K*)malloc(sizeof(K));
    expect_goto(p_key, fail_return);
    *p_key = key;
    to->callback.arg = p_key;

    value_wrap.to = to;
    value_wrap.value = val;
    this->table.insert({key, value_wrap});

    // add value to timing wheel
    timeouts_update(this->m_to, std::time(nullptr) * 1000);
    timeouts_add(m_to, to, 3000);
    return true;

fail_return:
    if (p_key) {
        free(p_key);
    }
    if (to) {
        free(to);
    }
    return false;
}

template<typename K, typename V>
bool cpt_hash_table<K, V>::del(const K &key) {
    typename std::map<K, value_wrap_t>::iterator itr;
    itr = this->table.find(key);
    if (itr != this->table.end()) {
        timeouts_del(this->m_to, itr->second.to);
        this->table.erase(key);
    }
    return true;
}

template<typename K, typename V>
bool cpt_hash_table<K, V>::find(const K &key, V &val) {
    typename std::map<K, value_wrap_t>::iterator itr;
    itr = this->table.find(key);
    if (itr != this->table.end()) {
        // update value in timing wheel
        timeouts_del(this->m_to, itr->second.to);
        timeouts_add(this->m_to, itr->second.to, 3000);
        val = itr->second.value;
        return true;
    } else {
        return false;
    }
}

template<typename K, typename V>
cpt_hash_table<K, V>::~cpt_hash_table() {
    // remove all value, and destroy timing wheel
    if (this->m_to) {
        struct timeout* to;
        TIMEOUTS_FOREACH(to, this->m_to, 0) {
            free(to->callback.arg);
            free(to);
        }
        timeouts_close(this->m_to);
    }
}

template<typename K, typename V>
void cpt_hash_table<K, V>::timeout_update() {
    // update timing wheel, time unit: ms
    timeouts_update(this->m_to, std::time(nullptr) * 1000);
    struct timeout* to ;
    while ((to = timeouts_get(this->m_to))) {
        this->table.erase(*((K*)(to->callback.arg)));
        free(to->callback.arg);
        free(to);
    }
}

#endif //CPP_TOOLKIT_MOD_HASH_TABLE_H
