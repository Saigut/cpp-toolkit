#include "app_asio_socket_server.h"

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_context.hpp>

#include <mod_common/log.h>
#include <mod_common/expect.h>

using boost::asio::ip::tcp;
using boost::asio::ip::address;
using boost::asio::io_context;

static int asio_server()
{
    boost::system::error_code ec;

    // server endpoint
    boost::asio::ip::address addr = boost::asio::ip::make_address("::0", ec);
    check_ec_ret_val(ec, -1, "make_address");
    tcp::endpoint endpoint(addr, 12345);

    // acceptor
    io_context io_ctx;
    boost::asio::ip::tcp::acceptor::reuse_address reuse_address_option(true);
    tcp::acceptor acceptor(io_ctx);
    acceptor.open(endpoint.protocol(), ec);
    check_ec_ret_val(ec, -1, "acceptor open");
    acceptor.set_option(reuse_address_option, ec);
    check_ec_ret_val(ec, -1, "acceptor set_option");
    acceptor.bind(endpoint, ec);
    check_ec_ret_val(ec, -1, "acceptor bind");

    acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);
    check_ec_ret_val(ec, -1, "acceptor listen");

    // accept
    while (true) {
        boost::asio::ip::tcp::socket client_socket = acceptor.accept(ec);
        check_ec_ret_val(ec, -1, "acceptor accept");

        tcp::endpoint remote_ep = client_socket.remote_endpoint(ec);
        check_ec_ret_val(ec, -1, "failed to get remote_endpoint");

        log_info("New client, ip: %s, port: %u",
                remote_ep.address().to_string().c_str(),
                remote_ep.port());
    }

    return 0;
}


int app_asio_socket_server(int argc, char** argv)
{
    return asio_server();
}
