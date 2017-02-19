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
#include "common/cslock.h"
#include "common/ccondition.h"
#include "common/base_thread.h"
#include "common/IReference.h"

namespace cr_rpc
{
    template<typename T, typename K>
    class select_event
    {
    public:
        select_event(T class_ptr, K class_func)
            :   _class_ptr(class_ptr),
                _class_func(class_func)
        {
        }

    protected:
        T _class_ptr;
        K _class_func;
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

    template<typename T, typename K>
    class socket_accept_task : public select_task,
                        public select_event<T,K>
    {
    public:
        static socket_accept_task* new_instance(int listen_fd, T class_ptr, K class_func)
        {
            return new socket_accept_task(listen_fd, class_ptr, class_func);
        }

    private:
        socket_accept_task(int listen_fd, T class_ptr, K class_func)
            :   select_task(select_task::TASK_SELECT_ACCEPT, listen_fd),
                select_event<T,K>(class_ptr, class_func)
        {
        }

        virtual ~socket_accept_task(){}
        virtual bool done()
        {
            struct sockaddr_un client_addr;
            socklen_t addr_len = sizeof(client_addr);
            int client_fd = ::accept(_socket_fd, (struct sockaddr *)&client_addr, &addr_len);
            if (client_fd < 0)
            {
                ::perror("accept err");
                return false;
            }
            ((this->_class_ptr)->*(this->_class_func))(client_fd);

            return true;
        }
    };

    template<typename T, typename K>
    class fifo_accept_task : public select_task,
                             public select_event<T,K>
    {
    public:
        static fifo_accept_task* new_instance(int listen_fd, T class_ptr, K class_func)
        {
            return new fifo_accept_task(listen_fd, class_ptr, class_func);
        }

    private:
        fifo_accept_task(int listen_fd, T class_ptr, K class_func)
            :   select_task(select_task::TASK_SELECT_ACCEPT, listen_fd),
                select_event<T,K>(class_ptr, class_func)
        {
        }

        virtual ~fifo_accept_task(){}
        virtual bool done()
        {
            char read_buf[512] = {0};
            ssize_t size = ::read(_socket_fd, read_buf, sizeof read_buf);
            ((this->_class_ptr)->*(this->_class_func))(read_buf, size);

            return true;
        }
    };

    template<typename T, typename K>
    class read_task : public select_task,
                      public select_event<T,K>
    {
    public:
        static read_task* new_instance(int read_fd, T class_ptr, K class_func)
        {
            return new read_task(read_fd, class_ptr, class_func);
        }

    private:
        read_task(int read_fd, T class_ptr, K class_func)
            :   select_task(select_task::TASK_SELECT_READ, read_fd),
                select_event<T,K>(class_ptr, class_func)
        {

        }

        virtual ~read_task(){}
        virtual bool done()
        {
            char buf[1024] = {0};
            size_t size = 0;
            do
            {
                size = ::read(_socket_fd, buf, sizeof(buf));
                ((this->_class_ptr)->*(this->_class_func))(_socket_fd, buf, size);
            }while(size == sizeof(buf));

            return true;
        }
    };

    template<typename T, typename K>
    class write_task : public select_task,
                       public select_event<T,K>
    {
    public:
        static write_task* new_instance(int write_fd, T class_ptr, K class_func, const char* buf, size_t size)

        {

            return new write_task(write_fd, class_ptr, class_func, buf, size);
        }

    private:
        write_task(int write_fd, T class_ptr, K class_func, const char* buf, size_t size)
            :   select_task(select_task::TASK_SELECT_WRITE, write_fd),
        select_event<T,K>(class_ptr, class_func),
        _send_size(size)
        {
            _send_buf = new char[size];
            ::memcpy(_send_buf, buf, size);
        }
        virtual ~write_task(){}
        virtual bool done()
        {
            ssize_t size;
            do
            {
                size = ::write(_socket_fd, _send_buf, _send_size);
                if(size == -1)
                {
                    if (errno != EAGAIN && errno != EWOULDBLOCK)
                        ((this->_class_ptr)->*(this->_class_func))(_socket_fd, -1);

                    return false;
                }
                ((this->_class_ptr)->*(this->_class_func))(_socket_fd, size);
                _send_size -= size;
                _send_buf += size;
            }while(_send_size > 0 && size > 0);

            delete []_send_buf;

            return true;
        }

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
