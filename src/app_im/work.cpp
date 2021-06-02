#include <app_im/work.h>

/*
 * class Work
 */
void Work::do_my_part()
{
    if (stopped) {
        return;
    }
    if (!began) {
        began = true;
        class Work* t_this = this;
        m_wp = context::callcc([t_this](context::continuation && c){
            t_this->m_wp = std::move(c);
            t_this->do_work();
            t_this->stopped = true;
            t_this->consignor_add_self_back_to_main_worker();
            return std::move(t_this->m_wp);
        });
    } else {
        if (m_wp) {
            m_wp = m_wp.resume();
        }
    }
}

void Work::set_my_worker(Worker* main_worker)
{
    m_main_worker = main_worker;
}

void Work::consignor_add_self_back_to_main_worker()
{
    if (m_consignor) {
        m_consignor->add_self_back_to_main_worker();
        m_consignor = nullptr;
    }
}

void Work::do_work()
{
}

void Work::add_self_back_to_main_worker()
{
    m_main_worker->add_work(this);
}

/*
 * class Work_Other
 */
void Work_Other::do_work()
{
    log_info("before do 2");
    std::this_thread::sleep_for(std::chrono::seconds (3));
    log_info("after do 2");
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
