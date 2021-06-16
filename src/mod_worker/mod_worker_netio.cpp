#include <mod_worker/mod_worker_netio.h>

#include <mod_common/expect.h>
#include <asio/io_context.hpp>
#include <asio/deadline_timer.hpp>

using boost::asio::ip::tcp;
using boost::asio::ip::address;
using boost::asio::io_context;

int Work_NetTcpConnect::do_my_part(io_context &io_ctx)
{
    std::shared_ptr<Work> this_obj = shared_from_this();
    m_socket_to_server->async_connect(m_endpoint,
                                     [this_obj](const boost::system::error_code& ec)
                                     {
                                         check_ec(ec, "connect");
                                         this_obj->m_my_finish_ret_val = ec ? -1 : 0;
                                         this_obj->finish_handler();
                                     });
    return 0;
}

int Work_NetTcpAccept::do_my_part(io_context &io_ctx)
{
    boost::system::error_code ec;
    std::shared_ptr<Work> this_obj = shared_from_this();
    if (!m_acceptor) {
        m_acceptor = std::make_shared<tcp::acceptor>(io_ctx);
        tcp::acceptor::reuse_address reuse_address_option(true);
        m_acceptor->open(m_bind_endpoint.protocol(), ec);
        check_ec_goto(ec, fail_return, "acceptor open");
        m_acceptor->set_option(reuse_address_option, ec);
        check_ec_goto(ec, fail_return, "acceptor set_option");
        m_acceptor->listen(boost::asio::socket_base::max_listen_connections, ec);
        check_ec_goto(ec, fail_return, "acceptor listen");
    }
    m_acceptor->async_accept([this_obj](
            const boost::system::error_code& ec,
            tcp::socket peer)
    {
        check_ec(ec, "accept");
        if (!ec) {
            ((Work_NetTcpAccept*)(this_obj.get()))->m_socket_to_a_client = std::make_shared<tcp::socket>(std::move(peer));
        }
        this_obj->m_my_finish_ret_val = ec ? -1 : 0;
        this_obj->finish_handler();
    });

    return 0;

fail_return:
    return -1;
}


int Work_NetTcpIn::do_my_part(io_context &io_ctx)
{
    std::shared_ptr<Work> this_obj = shared_from_this();
    m_socket->async_read_some(in_buf, [this_obj](
            const boost::system::error_code& ec,
            std::size_t read_b_num)
    {
        check_ec(ec, "read_some");
        this_obj->m_my_finish_ret_val = ec ? -1 : 0;
        this_obj->finish_handler();
    });
    return 0;
}

int Work_NetTcpOut::do_my_part(io_context &io_ctx)
{
    std::shared_ptr<Work> this_obj = shared_from_this();
    m_socket->async_write_some(out_buf, [this_obj](
            const boost::system::error_code& ec,
            std::size_t write_b_num)
    {
        check_ec(ec, "write_some");
        if (!ec) {
//            log_info("wrote bytes: %zu", write_b_num);
//            printf("wrote bytes: %zu\n", write_b_num);
        }
        this_obj->m_my_finish_ret_val = ec ? -1 : 0;
        this_obj->finish_handler();
    });
    return 0;
}

int Work_TimerWaitUntil::do_my_part(io_context &io_ctx)
{
    return 0;
}

int Work_TimerWaitFor::do_my_part(io_context &io_ctx)
{
    return 0;
}

int Work_TimerCancel::do_my_part(io_context &io_ctx)
{
    return 0;
}

//void Worker_NetIo::pop_queue(boost::posix_time::microseconds interval)
//{
//    Worker_NetIo* this_obj = this;
//    boost::asio::deadline_timer timer(m_io_ctx, interval);
//
//    auto timeout_func = [this_obj](const boost::system::error_code& ec) {
//        boost::posix_time::microseconds interval_1(1);
//        boost::posix_time::microseconds interval_0(0);
//        auto cur_work = (Work_NetIo_Asio*)this_obj->get_cur_work();
//        if (cur_work) {
//            cur_work->do_my_part(this_obj->m_io_ctx);
//            this_obj->pop_queue(interval_0);
//        } else {
//            this_obj->pop_queue(interval_1);
//        }
//    };
//    timer.expires_at(timer.expires_at() + interval);
//    timer.async_wait(timeout_func);
//}

void Worker_NetIo::run()
{
    boost::asio::io_context::work io_work(m_io_ctx);
//    boost::posix_time::microseconds interval_0(0);
//    pop_queue(interval_0);
    m_io_ctx.run();
}

int Worker_NetIo::add_work(std::shared_ptr<Work_NetIo_Asio> work)
{
    return work->do_my_part(m_io_ctx);
}

void Worker_NetIo::wait_worker_started()
{
    while (m_io_ctx.stopped()) {
        std::this_thread::sleep_for(std::chrono::microseconds(1));
    }
}
