#include "app_prod_im_internal.h"


/*
* 登录会话
  * 开启会话
    login_session_open，参数：IO连接 socket；返回值：结果 int
  * 关闭会话
    login_session_close
  * 获取IO连接
    login_session_get_io_port，参数：无；返回值：IO连接 socket
*/

int prod_im_c_mod_login_session::open(int io_skt)
{
    return -1;
}

void prod_im_c_mod_login_session::close()
{

}

int prod_im_c_mod_login_session::get_io_port()
{
    return -1;
}
