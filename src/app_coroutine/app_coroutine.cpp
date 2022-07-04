#include <app_coroutine/app_coroutine.h>

#include <thread>
#include <boost/asio/io_context.hpp>
#include <boost/lexical_cast.hpp>
#include <mod_common/log.h>
#include <mod_coroutine/mod_coroutine.h>
#include <mod_coroutine/mod_co_net.h>

using boost::asio::io_context;

static void asio_thread(io_context& io_ctx)
{
    util_bind_thread_to_core(2);
    boost::asio::io_context::work io_work(io_ctx);
    io_ctx.run();
    log_info("Asio io context quit!!");
}

void my_co_net(io_context& io_ctx)
{
    cppt_co_tcp_socket_builder builder(io_ctx);
    auto peer_socket = builder.connect("127.0.0.1", 80);
    if (!peer_socket) {
        log_error("connect failed!");
        return;
    }

    char send_buf[4096];
    char recv_buf[4096] = { 0 };
    ssize_t ret_recv;
    snprintf(send_buf, sizeof(send_buf), "GET / HTTP/1.1\r\n"
                                         "Host: 127.0.0.1\r\n"
                                         "Connection: keep-alive\r\n"
                                         "User-Agent: curl/7.58.0\r\n"
                                         "Accept: */*\r\n\r\n");
    // Send
    expect_goto(peer_socket->write((uint8_t*)send_buf, std::strlen(send_buf)), func_return);

    // Recv
    ret_recv = peer_socket->read_some((uint8_t*)recv_buf, sizeof(recv_buf));
    expect_goto(ret_recv > 0, func_return);

    recv_buf[ret_recv < sizeof(recv_buf) ? ret_recv : sizeof(recv_buf) - 1] = 0;
    log_info("received:\n%s!", recv_buf);

func_return:
    return;
}

void my_co_net_main(io_context& io_ctx)
{
    for (int i = 0; i < 50; i++) {
        cppt_co_create(my_co_net, std::ref(io_ctx));
//        auto id = cppt_co_awaitable_create(my_co_net, std::ref(io_ctx));
//        cppt_co_await(id);
    }
}

void test_http_client()
{
    io_context io_ctx;
    std::thread asio_thr{asio_thread, std::ref(io_ctx)};
    asio_thr.detach();
    cppt_co_create(my_co_net_main, std::ref(io_ctx));
    cppt_co_main_run();
}

struct header
{
    std::string name;
    std::string value;
};

struct request
{
    /// The request method, e.g. "GET", "POST".
    std::string method;

    /// The requested URI, such as a path to a file.
    std::string uri;

    /// Major version number, usually 1.
    int http_version_major;

    /// Minor version number, usually 0 or 1.
    int http_version_minor;

    /// The headers included with the request.
    std::vector<header> headers;

    /// The optional content sent with the request.
    std::string content;
};

class read_char_t {
public:
    explicit read_char_t(cppt_co_tcp_socket& tcp_socket)
    : m_tcp_socket(tcp_socket) {}
    char read_char() {
        if (m_data_size > 0 && (m_read_pos < m_data_size)) {
            return m_recv_buf[m_read_pos++];
        }
        m_data_size = 0;
        m_read_pos = 0;
        ssize_t read_ret = m_tcp_socket.read_some((uint8_t*)m_recv_buf, sizeof(m_recv_buf));
        if (read_ret <= 0) {
            return '\0';
        } else {
            m_data_size = read_ret;
            return read_char();
        }
    }
private:
    cppt_co_tcp_socket& m_tcp_socket;
    char m_recv_buf[4096] = { 0 };
    size_t m_data_size = 0;
    size_t m_read_pos = 0;
};

bool is_char(int c)
{
    return c >= 0 && c <= 127;
}

bool is_ctl(int c)
{
    return (c >= 0 && c <= 31) || (c == 127);
}

bool is_tspecial(int c)
{
    switch (c)
    {
        case '(': case ')': case '<': case '>': case '@':
        case ',': case ';': case ':': case '\\': case '"':
        case '/': case '[': case ']': case '?': case '=':
        case '{': case '}': case ' ': case '\t':
            return true;
        default:
            return false;
    }
}

bool is_digit(int c)
{
    return c >= '0' && c <= '9';
}

bool tolower_compare(char a, char b)
{
    return std::tolower(a) == std::tolower(b);
}

bool headers_equal(const std::string& a, const std::string& b)
{
    if (a.length() != b.length())
        return false;

    return std::equal(a.begin(), a.end(), b.begin(),
                      &tolower_compare);
}

std::string content_length_name_ = "Content-Length";

#define read_in_char() do {c = read_char.read_char(); if ('\0' == c) return -1;} while(false)

// ret: 0 finished; -1 error.
int consume_header(request& req, cppt_co_tcp_socket& tcp_socket)
{
    char c;
    size_t content_length_ = 0;
    read_char_t read_char{tcp_socket};
    read_in_char();
    // Request method.
    while (is_char(c) && !is_ctl(c) && !is_tspecial(c) && c != ' ')
    {
        req.method.push_back(c);
        read_in_char();
    }
    if (req.method.empty())
        return -1;

    // Space.
    if (c != ' ') return -1;
    read_in_char();

    // URI.
    while (!is_ctl(c) && c != ' ')
    {
        req.uri.push_back(c);
        read_in_char();
    }
    if (req.uri.empty()) return -1;

    // Space.
    if (c != ' ') return -1;
    read_in_char();

    // HTTP protocol identifier.
    if (c != 'H') return -1;
    read_in_char();
    if (c != 'T') return -1;
    read_in_char();
    if (c != 'T') return -1;
    read_in_char();
    if (c != 'P') return -1;
    read_in_char();

    // Slash.
    if (c != '/') return -1;
    read_in_char();

    // Major version number.
    if (!is_digit(c)) return -1;
    while (is_digit(c))
    {
        req.http_version_major = req.http_version_major * 10 + c - '0';
        read_in_char();
    }

    // Dot.
    if (c != '.') return -1;
    read_in_char();

    // Minor version number.
    if (!is_digit(c)) return -1;
    while (is_digit(c))
    {
        req.http_version_minor = req.http_version_minor * 10 + c - '0';
        read_in_char();
    }

    // CRLF.
    if (c != '\r') return -1;
    read_in_char();
    if (c != '\n') return -1;
    read_in_char();

    // Headers.
    while ((is_char(c) && !is_ctl(c) && !is_tspecial(c) && c != '\r')
           || (c == ' ' || c == '\t'))
    {
        if (c == ' ' || c == '\t')
        {
            // Leading whitespace. Must be continuation of previous header's value.
            if (req.headers.empty()) return -1;
            while (c == ' ' || c == '\t') {
                read_in_char();
            }
        }
        else
        {
            // Start the next header.
            req.headers.push_back(header());

            // Header name.
            while (is_char(c) && !is_ctl(c) && !is_tspecial(c) && c != ':')
            {
                req.headers.back().name.push_back(c);
                read_in_char();
            }

            // Colon and space separates the header name from the header value.
            if (c != ':') return -1;
            read_in_char();
            if (c != ' ') return -1;
            read_in_char();
        }

        // Header value.
        while (is_char(c) && !is_ctl(c) && c != '\r')
        {
            req.headers.back().value.push_back(c);
            read_in_char();
        }

        // CRLF.
        if (c != '\r') return -1;
        read_in_char();
        if (c != '\n') return -1;
        read_in_char();
    }

    // CRLF.
    if (c != '\r') return -1;
    read_in_char();
    if (c != '\n') return -1;

    // Check for optional Content-Length header.
    for (auto & header : req.headers)
    {
        if (headers_equal(header.name, content_length_name_))
        {
            try
            {
                content_length_ =
                        boost::lexical_cast<size_t>(header.value);
            }
            catch (boost::bad_lexical_cast&)
            {
                return -1;
            }
        }
    }

    // Content.
    while (req.content.size() < content_length_)
    {
        read_in_char();
        req.content.push_back(c);
    }

    return 0;
}

struct log_record {
    uint64_t t1;
    uint64_t t2;
    uint64_t t3;
    uint64_t t4;
    uint64_t t5;
    uint64_t t6;
};

void print_log_record(log_record& log)
{
    log_info("t1: %lluus", log.t1);
    log_info("t2: %lluus", log.t2);
    log_info("t3: %lluus", log.t3);
    log_info("t4: %lluus", log.t4);
    log_info("t5: %lluus", log.t5);
    log_info("t6: %lluus", log.t6);
}

void co_http_server_process_request(std::shared_ptr<cppt_co_tcp_socket> tcp_socket,
                                    log_record& logs)
{
    request req;

    logs.t2 = util_now_ts_us();
    if (0 == consume_header(req, *tcp_socket)) {
//        log_info("Valid request!");
        logs.t3 = util_now_ts_us();
        const char* res_str = "HTTP/1.1 200 OK\r\n"
                              "Server: cppt co http/0.1.0\r\n"
                              "Content-Length: 7\r\n"
                              "Content-Type: text/html; charset=utf-8\r\n"
                              "Last-Modified: Wed, 06 Jan 2021 06:15:08 GMT\r\n"
                              "Connection: close\r\n"
                              "\r\n"
                              "Hello!\n";
        tcp_socket->write((uint8_t*)res_str, strlen(res_str));
        logs.t4 = util_now_ts_us();
        tcp_socket->close();
        logs.t5 = util_now_ts_us();
//        print_log_record(logs);
    } else {
        log_error("Invalid request!");
        tcp_socket->close();
    }
}

void co_http_server_main(io_context& io_ctx)
{
    log_record logs;
    cppt_co_tcp_socket_builder builder(io_ctx);
    expect_ret(builder.listen("0.0.0.0", 10666));
    while (true) {
        std::shared_ptr<cppt_co_tcp_socket> peer_socket = builder.accept();
        if (!peer_socket) {
            return;
        }
        logs.t1 = util_now_ts_us();
        cppt_co_create(co_http_server_process_request, peer_socket, std::ref(logs));
    }
}

void test_http_server()
{
    io_context io_ctx;
    std::thread asio_thr{ asio_thread, std::ref(io_ctx) };
    asio_thr.detach();
    cppt_co_create(co_http_server_main, std::ref(io_ctx));
    util_bind_thread_to_core(3);
    cppt_co_main_run();
}

int app_coroutine(int argc, char** argv)
{
//    test_http_client();
    test_http_server();
    return 0;
}
