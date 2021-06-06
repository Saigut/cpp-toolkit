#ifndef CPP_TOOLKIT_WORKER_EPOLL_H
#define CPP_TOOLKIT_WORKER_EPOLL_H

#ifdef _CPPT_OS_LINUX

class Worker_Net : public Worker {
public:
    void run() override;
    int add_work(Work* work) override { exit(-1); return -1; };
    int add_avail_work(Work_GetStdin* work);
    int add_avail_work(Work_NetIn* work);
    int add_avail_work(Work_NetOut* work);
private:
    int m_epoll_fd = -1;
};

class Work_ImSend : public Work {
public:
    Work_ImSend() = default;
    explicit Work_ImSend(Worker_Net* other_worker)
            : m_other_worker(other_worker)
    {}
private:
    void do_work() override;
//    Worker* m_other_worker = nullptr;
    Worker_Net* m_other_worker = nullptr;
};

class Work_GetStdin : public Work {
public:
    Work_GetStdin() = default;
    explicit Work_GetStdin(Work* consignor)
            : Work(consignor)
    {}
    void do_my_part() override;
    uint8_t* data_buf = nullptr;
};

class Work_NetIn : public Work {
public:
    Work_NetIn() = default;
    explicit Work_NetIn(Work* consignor)
            : Work(consignor)
    {}
    void do_my_part() override;
    buf_wrap in_buf = {nullptr, 0};
    int fd = -1;
};

class Work_NetOut : public Work {
public:
    Work_NetOut() = default;
    explicit Work_NetOut(Work* consignor)
            : Work(consignor)
    {}
    void do_my_part() override;
    buf_wrap out_buf = {nullptr, 0};
    int fd = -1;
};

#endif //_CPPT_OS_LINUX

#endif //CPP_TOOLKIT_WORKER_EPOLL_H
