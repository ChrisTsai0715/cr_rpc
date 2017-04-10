#include "select_task.h"
using namespace cr_rpc;
select_tracker::select_tracker()
    :   _stop_flag(false)
{

}

bool select_tracker::add_task(CRefObj<select_task> task)
{
    cr_common::auto_cond cscond(_task_cond);
    _uncomplete_task_lists.push_back(task);
    return _task_cond.signal() == 0;
}

bool select_tracker::del_task(CRefObj<select_task> task)
{
    cr_common::auto_cond cscond(_task_cond);
    _uncomplete_task_lists.remove(task);
    return true;
}

bool select_tracker::del_task(int socket_fd, int task_type)
{
    cr_common::auto_cond cscond(_task_cond);
    select_task_list_type task_lists = _uncomplete_task_lists;
    for (select_task_list_type::iterator it = task_lists.begin();
         it != task_lists.end();
         it ++)
    {
        if ((*it)->get_socket_fd() == socket_fd && (*it)->get_task_type() == task_type)
        {
            _uncomplete_task_lists.remove(*it);
        }
    }
    return false;
}

bool select_tracker::thread_loop()
{
    fd_set read_sockets;
    fd_set write_sockets;
    fd_set err_sockets;
    int max_fd = -1;
    while(1)
    {
        if (_stop_flag)
        {
            return true;
        }

        FD_ZERO(&read_sockets);
        FD_ZERO(&write_sockets);
        FD_ZERO(&err_sockets);

        select_task_list_type task_lists;
        {
            cr_common::auto_cond cscond(_task_cond);
            if (_uncomplete_task_lists.size() == 0)
            {
                _task_cond.time_wait(1000);
                if (_uncomplete_task_lists.size() == 0)
                    continue;
            }
            task_lists = _uncomplete_task_lists;
        }

        for (select_task_list_type::iterator it = task_lists.begin();
             it != task_lists.end();
             it ++)
        {
            switch((*it)->get_task_type())
            {
            case select_task::TASK_SELECT_READ:
            case select_task::TASK_SELECT_ACCEPT:
                FD_SET((*it)->get_socket_fd(), &read_sockets);
                break;

            case select_task::TASK_SELECT_WRITE:
                FD_SET((*it)->get_socket_fd(), &write_sockets);
                break;

            default:
                break;
            }

            max_fd = (*it)->get_socket_fd() > max_fd ? (*it)->get_socket_fd() : max_fd;
        }

        struct timeval time_out;
        time_out.tv_sec  = 1;
        time_out.tv_usec = 0;
        int ret = select(max_fd + 1, &read_sockets, &write_sockets, NULL, &time_out);
        if (ret == -1)
        {
            return false;
        }
        else if (ret > 0)
        {
            for (select_task_list_type::iterator it = task_lists.begin();
                 it != task_lists.end();
                 it ++)
            {

                switch((*it)->get_task_type())
                {
                case select_task::TASK_SELECT_READ:
                case select_task::TASK_SELECT_ACCEPT:
                    if (FD_ISSET((*it)->get_socket_fd(), &read_sockets))
                    {
                        if ((*it)->done())
                            this->del_task(*it);
                    }
                    break;

                case select_task::TASK_SELECT_WRITE:
                    if (FD_ISSET((*it)->get_socket_fd(), &write_sockets))
                    {
                        if((*it)->done())
                            this->del_task(*it);
                    }
                    break;

                default:
                    break;
                }
            }
        }
    }
}

bool socket_accept_task::done()
{
    struct sockaddr_un client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_fd = ::accept(_socket_fd, (struct sockaddr *)&client_addr, &addr_len);
    if (client_fd < 0)
    {
        ::perror("accept err");
        return false;
    }

    comm_server_listener* listener = _get_listener<comm_server_listener>();
    if (listener != NULL) listener->_client_connect(client_fd);
    return true;
}

bool fifo_accept_task::done()
{
    char read_buf[512] = {0};
    ssize_t size = ::read(_socket_fd, read_buf, sizeof read_buf);

    comm_server_listener* listener = _get_listener<comm_server_listener>();
    if (listener != NULL) listener->_data_receive(_socket_fd, read_buf, size);
    return true;
}

bool read_task::done()
{
    char buf[1024] = {0};
    size_t size = 0;
    do
    {
        size = ::read(_socket_fd, buf, sizeof(buf));
        if (_listener) _listener->_data_receive(_socket_fd, buf, size);
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
            if (errno != EAGAIN && errno != EWOULDBLOCK)
                if (_listener) _listener->_data_send(_socket_fd, -1);

            return false;
        }
        if (_listener) _listener->_data_send(_socket_fd, size);
        _send_size -= size;
        _send_buf += size;
    }while(_send_size > 0 && size > 0);

    delete []_send_buf;

    return true;
}

bool socket_connect_task::done()
{
    if (_dest_addr_unix.size() != 0)
    {
        struct sockaddr_un connect_addr;
        bzero((void *)&connect_addr, sizeof(connect_addr));
        connect_addr.sun_family = AF_UNIX;
        strcpy(connect_addr.sun_path, _dest_addr_unix.c_str());

        int ret = ::connect(_socket_fd, (struct sockaddr *)&connect_addr, sizeof(connect_addr));
        if (ret < 0)
        {
            ::perror("connect err");
            return false;
        }
    }
    else
    {
        struct sockaddr_in connect_addr;
        connect_addr.sin_family = AF_INET;
        connect_addr.sin_port = htons(_port);
        connect_addr.sin_addr.s_addr = (in_addr_t)_dest_addr;
        socklen_t len = sizeof(struct sockaddr_in);
        int ret = ::connect(_socket_fd, (struct sockaddr *)&connect_addr, len);
        if (ret < 0)
        {
            ::perror("connect err");
            return false;
        }
   }

   comm_client_listener* listener = _get_listener<comm_client_listener>();
   if (listener != NULL) listener->_connect(_socket_fd);

   return true;
}
