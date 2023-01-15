#include "app_asio_socket_client.hpp"

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/algorithm/hex.hpp>

#include <mod_common/log.hpp>
#include <mod_common/expect.hpp>

using boost::asio::ip::tcp;
using boost::asio::ip::address;
using boost::asio::io_context;

static int asio_client()
{
    boost::system::error_code ec;

    // server endpoint
    boost::asio::ip::address addr = boost::asio::ip::make_address("127.0.0.1", ec);
    check_ec_ret_val(ec, -1, "make_address");
    tcp::endpoint endpoint(addr, 12345);

    io_context io_ctx;
    boost::asio::ip::tcp::socket client_socket{io_ctx};
    client_socket.connect(endpoint, ec);
    check_ec_ret_val(ec, -1, "client_socket connect");

    std::string send_str= "I'm client";
    boost::asio::const_buffer send_buf_wrap{send_str.c_str(), send_str.length()};
    client_socket.write_some(send_buf_wrap, ec);
    check_ec(ec, "write_some");

    size_t n;
    char recv_buf[1024];
    boost::asio::mutable_buffer recv_buf_wrap{recv_buf, sizeof(recv_buf)};
    n = client_socket.read_some(recv_buf_wrap, ec);
    check_ec(ec, "read_some");
    std::string recv_str{recv_buf, n};
    log_info("read: %s", recv_str.c_str());

    std::string hex_str;
    boost::algorithm::hex_lower(recv_str, std::back_inserter(hex_str));
    log_info("read, hex: %s", hex_str.c_str());

    return 0;
}

int app_asio_socket_client(int argc, char** argv)
{
    return asio_client();
}
