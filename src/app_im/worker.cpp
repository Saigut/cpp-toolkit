#include <app_im/worker.h>

#include <mod_common/expect.h>

/*
 * class Worker
 */
void Worker::run()
{
    while(true) {
        if (0 != do_cur_work()) {
            std::this_thread::sleep_for(std::chrono::nanoseconds(1));
        }
    }
}

int Worker::add_work(Work* work)
{
    works_q.push(work);
    work->set_my_worker(this);
    return 0;
}

int Worker::do_cur_work()
{
    Work* cur_work = get_cur_work();
    if (cur_work) {
        cur_work->do_my_part();
        return 0;
    } else {
        return -1;
    }
}

Work* Worker::get_cur_work()
{
    Work* cur_work;
    if (works_q.pop(cur_work)) {
        return cur_work;
    } else {
        return nullptr;
    }
}

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
