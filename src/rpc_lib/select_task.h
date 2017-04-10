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

namespace cr_rpc
{
    class comm_base_listener;
    class comm_server_listener;
    class comm_client_listener;

    class select_event
    {
    public:
        select_event(comm_base_listener* listener)
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

    protected:
        comm_base_listener* _listener;
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

    class socket_accept_task : public select_task,
                        public select_event
    {
    public:
        static socket_accept_task* new_instance(int listen_fd, comm_base_listener* listener)
        {
            return new socket_accept_task(listen_fd, listener);
        }

    private:
        socket_accept_task(int listen_fd, comm_base_listener* listener)
            :   select_task(select_task::TASK_SELECT_ACCEPT, listen_fd),
                select_event(listener)
        {
        }

        virtual ~socket_accept_task(){}
        virtual bool done();
    };

    class fifo_accept_task : public select_task,
                             public select_event
    {
    public:
        static fifo_accept_task* new_instance(int listen_fd, comm_base_listener* listener)
        {
            return new fifo_accept_task(listen_fd, listener);
        }

    private:
        fifo_accept_task(int listen_fd, comm_base_listener* listener)
            :   select_task(select_task::TASK_SELECT_ACCEPT, listen_fd),
                select_event(listener)
        {
        }

        virtual ~fifo_accept_task(){}
        virtual bool done();
    };

    class socket_connect_task : public select_task,
                                public select_event
    {
    public:
        static socket_connect_task* new_instance(int connect_fd, comm_base_listener* listener, uint32_t dest_addr, uint16_t port)
        {
            return new socket_connect_task(connect_fd, listener, dest_addr, port);
        }

    private:
        socket_connect_task(int connect_fd, comm_base_listener* listener, uint32_t dest_addr, uint16_t port)
            :   select_task(select_task::TASK_SELECT_ACCEPT, connect_fd),
                select_event(listener),
                _dest_addr(dest_addr),
                _port(port)
        {
        }

        socket_connect_task(int connect_fd, comm_base_listener* listener, const char* dest_addr)
            :   select_task(select_task::TASK_SELECT_ACCEPT, connect_fd),
                select_event(listener),
                _dest_addr_unix(dest_addr)
        {
        }

        virtual ~socket_connect_task(){}
        virtual bool done();
    private:
        uint32_t _dest_addr;
        uint16_t _port;
        std::string _dest_addr_unix;
    };

    class read_task : public select_task,
                      public select_event
    {
    public:
        static read_task* new_instance(int read_fd, comm_base_listener* listener)
        {
            return new read_task(read_fd, listener);
        }

    private:
        read_task(int read_fd, comm_base_listener* listener)
            :   select_task(select_task::TASK_SELECT_READ, read_fd),
                select_event(listener)
        {

        }

        virtual ~read_task(){}
        virtual bool done();
    };

    class write_task : public select_task,
                       public select_event
    {
    public:
        static write_task* new_instance(int write_fd, comm_base_listener* listener, const char* buf, size_t size)
        {
            return new write_task(write_fd, listener, buf, size);
        }

    private:
        write_task(int write_fd, comm_base_listener* listener, const char* buf, size_t size)
            :   select_task(select_task::TASK_SELECT_WRITE, write_fd),
        select_event(listener),
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

    class select_tracker : private base_thread
    {
    public:
        select_tracker();
        bool add_task(CRefObj<select_task> task);
        bool del_task(CRefObj<select_task> task);
        bool del_task(int socket_fd, int task_type);

        virtual bool run()
        {
            _stop_flag = false;
            return base_thread::run();
        }

        virtual bool stop()
        {
            _stop_flag = true;
            return base_thread::stop();
        }

    private:
        virtual bool thread_loop();

    private:
        typedef std::list<CRefObj<select_task> > select_task_list_type;
        cr_common::condition  _task_cond;
        select_task_list_type _uncomplete_task_lists;
        select_task_list_type _complete_task_lists;
        bool _stop_flag;
    };
}
#endif // SELECT_TASK_H
