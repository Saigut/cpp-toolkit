#ifndef CPP_TOOLKIT_PROD_IM_SERVER_MOD_USER_SESSION_HPP
#define CPP_TOOLKIT_PROD_IM_SERVER_MOD_USER_SESSION_HPP

#include <string>
#include <memory>
#include <map>
#include <boost/asio/deadline_timer.hpp>

struct prod_im_s_user_session {
    prod_im_s_user_session(const std::string& _user_id,
                           const std::string& _client_ip,
                           uint16_t _client_port,
                           std::shared_ptr<boost::asio::deadline_timer> _timer)
            : user_id(_user_id),
              client_ip(_client_ip),
              client_port(_client_port),
              timer(_timer) {}
    std::string user_id;
    std::string client_ip;
    uint16_t client_port;
    std::shared_ptr<boost::asio::deadline_timer> timer;
};

class prod_im_s_mod_user_session {
public:
    explicit prod_im_s_mod_user_session(boost::asio::io_context& io_ctx)
            : m_io_ctx(io_ctx) {}
    int add(const std::string& user_id, const std::string& client_ip, uint16_t client_port);
    void del(const std::string& user_id);
    std::shared_ptr<prod_im_s_user_session> find(const std::string& user_id);
private:
    std::map<std::string, prod_im_s_user_session> m_user_sessions;
    boost::asio::io_context& m_io_ctx;
};

#endif //CPP_TOOLKIT_PROD_IM_SERVER_MOD_USER_SESSION_HPP
