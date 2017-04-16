#include "select_task.h"
using namespace cr_common;

bool fifo_accept_task::done()
{
    char read_buf[512] = {0};
    ssize_t size = ::read(_socket_fd, read_buf, sizeof read_buf);

    comm_server_listener* listener = _get_listener<comm_server_listener>();
//    if (listener != NULL) listener->_data_receive(_socket_fd, read_buf, size);
    return true;
}

bool read_task::done()
{
    char buf[1024] = {0};
    size_t size = 0;
    do
    {
        size = ::read(_socket_fd, buf, sizeof(buf));
        //if (_listener) _listener->_on_data_receive(_socket_fd, buf, size);
    }while(size == sizeof(buf));

    return true;
}

bool write_task::done()
{
    ssize_t size;
    do
    {
        size = ::write(_socket_fd, _send_buf, _send_size);
        if(size == -1)
        {
//            if (errno != EAGAIN && errno != EWOULDBLOCK)
//                if (_listener) _listener->_on_data_send(_socket_fd, -1);

            return false;
        }
 //       if (_listener) _listener->_on_data_send(_socket_fd, size);
        _send_size -= size;
        _send_buf += size;
    }while(_send_size > 0 && size > 0);

    delete []_send_buf;

    return true;
}
