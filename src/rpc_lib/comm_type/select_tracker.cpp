#include "select_tracker.h"
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
