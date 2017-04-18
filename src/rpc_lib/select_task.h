#ifndef SELECT_TASK_H
#define SELECT_TASK_H

#include <list>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <netinet/in.h>
#include "common/cslock.h"
#include "common/ccondition.h"
#include "common/base_thread.h"
#include "common/IReference.h"

namespace cr_common
{
    class select_task : public c_ref
    {
    public:
        enum
        {
            TASK_SELECT_ACCEPT,
            TASK_SELECT_READ,
            TASK_SELECT_WRITE,
        };

        select_task(int type, int socket_fd)
            :   _task_type(type),
                _socket_fd(socket_fd)
        {
        }
        virtual ~select_task(){}

        virtual bool done() = 0;
        int get_task_type() const {return _task_type;}
        int get_socket_fd() const {return _socket_fd;}

    protected:
        int _task_type;
        int _socket_fd;
    };

#if 0
    class fifo_accept_task : public select_task,
                             public io_select_event
    {
    public:
        static fifo_accept_task* new_instance(int listen_fd, cr_common::io_async_listener* listener)
        {
            return new fifo_accept_task(listen_fd, listener);
        }

    private:
        fifo_accept_task(int listen_fd, cr_common::io_async_listener* listener)
            :   select_task(select_task::TASK_SELECT_ACCEPT, listen_fd),
                io_select_event(listener)
        {
        }

        virtual ~fifo_accept_task(){}
        virtual bool done();
    };
#endif

}
#endif // SELECT_TASK_H
