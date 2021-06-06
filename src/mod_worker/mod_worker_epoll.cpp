#ifdef _CPPT_OS_LINUX

#include "worker_epoll.h"


/*
 * class Worker_Net
 */
#include <sys/epoll.h>
#define MAX_EVENTS 100
#define READ_SIZE 10
void Worker_Net::run()
{
    struct epoll_event events[MAX_EVENTS];
    int fd_nums = -1;
    int i;
    Work* work;

    m_epoll_fd = epoll_create1(0);
    expect_ret(m_epoll_fd >= 0);

    while (true) {
        fd_nums = epoll_wait(m_epoll_fd, events, MAX_EVENTS, -1);
        for(i = 0; i < fd_nums; i++) {
            work = (Work*)(events[i].data.ptr);
            work->do_my_part();
            expect(0 == epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, 0, &(events[i])));
        }
    }

    fail_return:
    if (m_epoll_fd >= 0) {
        close(m_epoll_fd);
        m_epoll_fd = -1;
    }
}

int Worker_Net::add_avail_work(Work_GetStdin* work) {
    expect_ret_val(m_epoll_fd >= 0, -1);

    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.ptr = (void*)work;

    expect_ret_val(0 == epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, 0, &event), -1);

    return 0;
}

int Worker_Net::add_avail_work(Work_NetIn* work) {
    expect_ret_val(m_epoll_fd >= 0, -1);

    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.ptr = (void*)work;

    expect_ret_val(0 == epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, 0, &event), -1);

    return 0;
}

int Worker_Net::add_avail_work(Work_NetOut* work) {
    expect_ret_val(m_epoll_fd >= 0, -1);

    struct epoll_event event;
    event.events = EPOLLOUT;
    event.data.ptr = (void*)work;

    expect_ret_val(0 == epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, 0, &event), -1);

    return 0;
}

/*
 * class Work_ImSend
 */
void Work_ImSend::do_work()
{
    char* data_buf[256 + 1];
    log_info("do 1");
//    Work_Other other{this};
    Work_NetIn other{this};
    other.in_buf.buf = (uint8_t*)data_buf;
    other.in_buf.buf_sz = 256;
    other.fd = 0;
    m_other_worker->add_avail_work(&other);
    m_wp = m_wp.resume();
    if (other.in_buf.buf_sz > 0
        && other.in_buf.buf_sz <= 256) {
        other.in_buf.buf[other.in_buf.buf_sz] = 0;
        log_info("Got: %s!", other.in_buf.buf);
    }
    log_info("do 3");
    log_info("do 4");
}

/*
 * class Work_GetStdin
 */
void Work_GetStdin::do_my_part()
{
    size_t bytes_read;
    if (this->data_buf) {
        bytes_read = read(0, this->data_buf, 128);
        log_info("%zd bytes read.", bytes_read);
        this->data_buf[bytes_read] = '\0';
        log_info("Read '%s'", this->data_buf);
        this->consignor_add_self_back_to_main_worker();
    } else {
        log_info("No buffer to read.");
    }
}

/*
 * class Work_NetIn
 */
void Work_NetIn::do_my_part()
{
    ssize_t ret;
    ret = read(this->fd, this->in_buf.buf, this->in_buf.buf_sz);
    this->in_buf.buf_sz = ret;
    consignor_add_self_back_to_main_worker();
}

/*
 * class Work_NetOut
 */
void Work_NetOut::do_my_part()
{
    ssize_t ret;
    ret = write(this->fd, this->out_buf.buf, this->out_buf.buf_sz);
    this->out_buf.buf_sz = this->out_buf.buf_sz - ret;
    consignor_add_self_back_to_main_worker();
}

#endif //_CPPT_OS_LINUX
