#include "app_prod_im_internal.h"

#include <grpcpp/grpcpp.h>
#include <prod_im_server.grpc.pb.h>

/*
* 登录会话
  * 开启会话
    login_session_open，参数：IO连接 socket；返回值：结果 int
  * 关闭会话
    login_session_close
  * 获取IO连接
    login_session_get_io_port，参数：无；返回值：IO连接 socket
*/

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

int prod_im_c_mod_login_session::open()
{
    return -1;
}

void prod_im_c_mod_login_session::close()
{

}

std::string prod_im_c_mod_login_session::get_server_ip()
{
    return m_server_ip;
}
