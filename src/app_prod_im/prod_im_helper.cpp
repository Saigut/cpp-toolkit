#include "prod_im_helper.h"
#include "app_prod_im_internal.h"

int im_s_op_msg_mpool::init()
{
    return 0;
}

void im_s_op_msg_mpool::deinit()
{
    prod_im_s_mod_main_msg* p = nullptr;
    while ((p = (prod_im_s_mod_main_msg*)mempool::alloc())) {
        delete p;
    }
}

void* im_s_op_msg_mpool::alloc()
{
    void* p = nullptr;
    p = mempool::alloc();
    if (p) {
        return p;
    }

    p = new prod_im_s_mod_main_msg;
    if (!p) {
        log_error("Failed to alloc!");
        return nullptr;
    }

    bool increased_num = false;
    {
        std::lock_guard lock(m_lock);
        if (m_total_ele_num < m_max_ele_num) {
            m_total_ele_num++;
            increased_num = true;
        }
    }

    if (!increased_num) {
        log_error("No buffer to alloc!");
        return nullptr;
    }

    return p;
}

void im_s_op_msg_mpool::free(void* buf)
{
    mempool::free(buf);
}
