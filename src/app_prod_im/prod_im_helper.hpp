#ifndef CPP_TOOLKIT_PROD_IM_HELPER_HPP
#define CPP_TOOLKIT_PROD_IM_HELPER_HPP

#include <mutex>
#include <mod_mempool/mod_mempool.hpp>

class im_s_op_msg_mpool : public mempool {
public:
    explicit im_s_op_msg_mpool(size_t ele_num)
    : m_max_ele_num(ele_num), mempool() {}

    int init() override;
    void deinit() override;
    void* alloc() override;
    void free(void* buf) override;

private:
    size_t m_max_ele_num = 0;
    size_t m_total_ele_num = 0;
    std::mutex m_lock;
};

#endif //CPP_TOOLKIT_PROD_IM_HELPER_HPP
