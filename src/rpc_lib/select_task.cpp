#include "select_task.h"
using namespace cr_common;

#if 0
bool fifo_accept_task::done()
{
    char read_buf[512] = {0};
    ssize_t size = ::read(_socket_fd, read_buf, sizeof read_buf);

    comm_server_listener* listener = _get_listener<comm_server_listener>();
//    if (listener != NULL) listener->_data_receive(_socket_fd, read_buf, size);
    return true;
}
#endif


