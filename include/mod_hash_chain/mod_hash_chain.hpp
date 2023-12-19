#ifndef CPP_TOOLKIT_MOD_HASH_CHAIN_H
#define CPP_TOOLKIT_MOD_HASH_CHAIN_H

#include <stdint.h>
#include <stddef.h>
#include <vector>


struct cppt_hc_block {
    uint8_t pre_block_hash[32];
    size_t data_size = 0;
    uint8_t* data = nullptr;
};

class cppt_hash_chain {
public:
    int append_block(cppt_hc_block& block) {
        return -1;
    }
    std::vector<cppt_hc_block*> chain;
    std::vector<uint8_t*> block_body;
};

#endif //CPP_TOOLKIT_MOD_HASH_CHAIN_H
