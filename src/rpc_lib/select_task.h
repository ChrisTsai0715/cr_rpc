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
#include "comm_listener.h"
#include "cr_socket/net_socket.h"
#include "cr_socket/io_async_listener.h"
#include "cr_socket/io_async_listener.h"
#include "base_comm.h"

namespace cr_common
{
    class io_select_event
    {
    public:
        io_select_event(cr_common::io_async_listener* listener)
            :   _listener(listener)
        {
        }

    protected:
        template<typename T>
        T* _get_listener()
        {
            if (_listener == 0)
                return NULL;

            return dynamic_cast<T*>(_listener);
        }

    private:
        cr_common::io_async_listener* _listener;
    };

    class comm_select_event
    {
    public:
        comm_select_event(base_comm* listener)
            :	_listener(listener)
        {
        }

    protected:
        template<typename T>
        T* _get_listener()
        {
            if (_listener == 0)
                return NULL;

            return dynamic_cast<T*>(_listener);
        }

    private:
        base_comm* _listener;

    };

    class select_task : public CReference
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

    class read_task : public select_task,
                      public io_select_event
    {
    public:
        static read_task* new_instance(int read_fd, cr_common::io_async_listener* listener)
        {
            return new read_task(read_fd, listener);
        }

    private:
        read_task(int read_fd, cr_common::io_async_listener* listener)
            :   select_task(select_task::TASK_SELECT_READ, read_fd),
                io_select_event(listener)
        {

        }

        virtual ~read_task(){}
        virtual bool done();
    };

    class write_task : public select_task,
                       public io_select_event
    {
    public:
        static write_task* new_instance(int write_fd, cr_common::io_async_listener* listener, const char* buf, size_t size)
        {
            return new write_task(write_fd, listener, buf, size);
        }

    private:
        write_task(int write_fd, cr_common::io_async_listener* listener, const char* buf, size_t size)
            :   select_task(select_task::TASK_SELECT_WRITE, write_fd),
        io_select_event(listener),
        _send_size(size)
        {
            _send_buf = new char[size];
            ::memcpy(_send_buf, buf, size);
        }
        virtual ~write_task(){}
        virtual bool done();

    private:
        char*  _send_buf;
        size_t _send_size;
    };
}
#endif // SELECT_TASK_H
