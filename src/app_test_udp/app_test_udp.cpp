#include "app_test_udp/app_test_udp.hpp"

#include <string>
#include <boost/asio.hpp>
#include <mod_common/log.hpp>
#include <mod_coroutine/mod_cor_net.hpp>

using boost::asio::ip::udp;

class UDPServer {
public:
    UDPServer(boost::asio::io_context& ioContext, uint16_t port)
            : socketBuilder_(ioContext), socket_(socketBuilder_.get_socket4(port))
    {}

    void cor_startReceive()
    {
        std::string message = "Hello, UDP Client!";
        ssize_t bytesRead = socket_->read_some(recvBuffer_.data(), recvBuffer_.size());
        if (bytesRead > 0) {
            log_info("Received data: %s", std::string((const char*)recvBuffer_.data(), bytesRead).c_str());

            // Echo the received data back to the client
            socket_->write((uint8_t*)message.data(), message.size());
            log_info("Sent echo response.");

            cor_startReceive(); // Continue to receive data
        }
    }

private:

    cppt::cor_udp_socket_builder socketBuilder_;
    std::shared_ptr<cppt::cor_udp_socket_t> socket_;
    std::array<uint8_t, 1024> recvBuffer_;
};

int cor_test_udp_client()
{
    boost::asio::io_context& ioContext = cppt::cor_get_io_context();
    cppt::cor_udp_socket_builder socketBuilder(ioContext);
    auto socket = socketBuilder.get_socket4();

    std::string serverIP = "127.0.0.1"; // Server IP address
    uint16_t serverPort = 12345;        // Server port

    cppt::net_sock_addr_t serverAddress;
    serverAddress.addr.type = CPPT_NETADDR_TYPE_IP4;

    if (1 != inet_pton(AF_INET, serverIP.c_str(), serverAddress.addr.ip4)) {
        log_info("Invalid IPv4 address format: %s", serverIP.c_str());
        return -1;
    }

    serverAddress.port = serverPort;

    // Set the server endpoint for the socket
    socket->set_peer_endpoint(serverAddress);

    std::string message = "Hello, UDP Server!";
    uint8_t recvBuffer[1024];

    socket->write((uint8_t*)message.data(), message.size());
    ssize_t bytesRead = socket->read_some(recvBuffer, sizeof(recvBuffer));
    if (bytesRead > 0) {
        log_info("Received data: %s", std::string((const char*)recvBuffer, bytesRead).c_str());
    }

    return 0;
}

int app_test_udp_client(int argc, char** argv)
{
    cppt::cor_create(cor_test_udp_client);
    cppt::cor_run();
    return 0;
}

int app_test_udp_server(int argc, char** argv)
{
    boost::asio::io_context& ioContext = cppt::cor_get_io_context();
    UDPServer server(ioContext, 12345);
    cppt::cor_create(&UDPServer::cor_startReceive, &server);
    cppt::cor_run();
    return 0;
}


int app_test_udp(int argc, char** argv)
{
    if (argc < 2) {
        log_error("wrong parameters.");
        return -1;
    }

    if (0 == strcmp(argv[1], "c")) {
        return app_test_udp_client(argc - 1, argv + 1);
    } else if (0 == strcmp(argv[1], "s")) {
        return app_test_udp_server(argc - 1, argv + 1);
    } else {
        log_error("wrong parameters.");
        return -1;
    }
}