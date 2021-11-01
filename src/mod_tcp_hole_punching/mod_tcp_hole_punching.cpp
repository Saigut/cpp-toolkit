#include <mod_tcp_hole_punching/mod_tcp_hole_punching.h>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_context.hpp>

#include <mod_common/log.h>
#include <mod_common/expect.h>

#define CONNECT_TO_IP "14.215.177.38"
#define CONNECT_TO_PORT 80

using boost::asio::ip::tcp;
using boost::asio::ip::address;
using boost::asio::io_context;

static int mod_tcp_hole_test_client(int argc, char** argv)
{
    return -1;
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
    // 4. print address and port of incoming client

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
