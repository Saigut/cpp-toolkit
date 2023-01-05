#ifndef CPP_TOOLKIT_MOD_CO_MUTEX_HPP
#define CPP_TOOLKIT_MOD_CO_MUTEX_HPP

class cppt_co_mutex {
public:
    bool lock();
    bool try_lock();
    void unlock();
};

#endif //CPP_TOOLKIT_MOD_CO_MUTEX_HPP
