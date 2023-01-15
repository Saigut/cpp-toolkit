#include <mod_tcp_hole_punching/mod_tcp_hole_punching.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_context.hpp>

#include <mod_common/log.hpp>
#include <mod_common/expect.hpp>

#define CONNECT_TO_IP "14.215.177.38"
#define CONNECT_TO_PORT 80

using boost::asio::ip::tcp;
using boost::asio::ip::address;
using boost::asio::io_context;

static int mod_tcp_hole_test_client(int argc, char** argv)
{
    boost::system::error_code ec;

    // server endpoint
    boost::asio::ip::address addr = boost::asio::ip::make_address(argv[1], ec);
    check_ec_ret_val(ec, -1, "make_address");
    tcp::endpoint endpoint(addr, std::atoi(argv[2]));

    io_context io_ctx;
    boost::asio::ip::tcp::socket client_socket{io_ctx};
    client_socket.connect(endpoint, ec);
    check_ec_ret_val(ec, -1, "client_socket connect");
    log_info("connected!");

    return 0;
}

static int mod_tcp_hole_test_server(int argc, char** argv)
{
    boost::system::error_code ec;
    io_context io_ctx;

    // 1. connect to CONNECT_TO_IP/CONNECT_TO_PORT
    boost::asio::ip::address addr = boost::asio::ip::make_address(
            CONNECT_TO_IP, ec);
    check_ec_ret_val(ec, -1, "make_address");
    tcp::endpoint endpoint(addr, CONNECT_TO_PORT);
    boost::asio::ip::tcp::socket client_socket{io_ctx};
    client_socket.connect(endpoint, ec);
    check_ec_ret_val(ec, -1, "client_socket connect");

    // 2. print local socket ip and port
    tcp::endpoint local_ep = client_socket.local_endpoint();
    log_info("Local address: %s/%u",
             local_ep.address().to_string().c_str(),
             local_ep.port());

    // 3. listen and accept on this tcp port
    boost::asio::ip::tcp::acceptor::reuse_address reuse_address_option(true);
    tcp::acceptor acceptor(io_ctx);
    acceptor.open(local_ep.protocol(), ec);
    check_ec_ret_val(ec, -1, "acceptor open");
    acceptor.set_option(reuse_address_option, ec);
    check_ec_ret_val(ec, -1, "acceptor set_option");
    acceptor.bind(local_ep, ec);
    check_ec_ret_val(ec, -1, "acceptor bind");
    acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);
    check_ec_ret_val(ec, -1, "acceptor listen");
    log_info("listening!");

    // 4. print address and port of incoming client
    boost::asio::ip::tcp::socket incoming_client_socket = acceptor.accept(ec);
    check_ec_ret_val(ec, -1, "acceptor accept");
    tcp::endpoint remote_ep = incoming_client_socket.remote_endpoint(ec);
    check_ec_ret_val(ec, -1, "failed to get remote_endpoint");
    log_info("New client, ip: %s, port: %u",
             remote_ep.address().to_string().c_str(),
             remote_ep.port());

    return 0;
}

// program s
// program c <server_addr> <server_port>
int mod_tcp_hole_test(int argc, char** argv)
{
    if (argc < 2) {
        log_error("wrong parameters.");
        return -1;
    }

    if (0 == strcmp(argv[1], "c")) {
        return mod_tcp_hole_test_client(argc - 1, argv + 1);
    } else if (0 == strcmp(argv[1], "s")) {
        return mod_tcp_hole_test_server(argc - 1, argv + 1);
    } else {
        log_error("wrong parameters.");
        return -1;
    }
}
