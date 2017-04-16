#ifndef BASE_COMM_H
#define BASE_COMM_H

#include <unistd.h>
#include "common/IReference.h"
#include "select_tracker.h"
#include "comm_listener.h"

namespace cr_common
{
    class base_comm : public CReference
    {
    public:
        base_comm()
            : 	_listener(0)
        {}
        explicit base_comm(comm_base_listener* listener);
        virtual ~base_comm();

    public:
        virtual ssize_t send_data(const char*, size_t) = 0;
        virtual ssize_t recv_data(char*, size_t) = 0;

    public:
        virtual void _on_data_receive(char*, ssize_t) = 0;
        virtual void _on_data_send(ssize_t) = 0;

    protected:
        CMutexLockEx   _read_mutex;
        CMutexLockEx   _write_mutex;

    protected:
        comm_base_listener* _listener;
    };

    class base_comm_client : public base_comm
    {
    public:
        base_comm_client(){}
        explicit base_comm_client(comm_client_listener* listener)
            :	base_comm(listener)
        {

        }

        virtual ~base_comm_client()
        {

        }

        virtual int start_connect(const std::string& path, unsigned int timeout_ms) = 0;
        virtual int disconnect_server() = 0;

    public:
        //interface for select task
        virtual void _on_connect() = 0;
        virtual void _on_disconnect() = 0;
    };

    class base_comm_server : public base_comm
    {
    public:
        base_comm_server(){}
        explicit base_comm_server(comm_server_listener* listener)
            :	base_comm(listener)
        {

        }

        virtual ~base_comm_server()
        {

        }

        virtual int start_listen(const std::string&) = 0;
        virtual int stop_listen() = 0;

    protected:
        virtual int _accept() = 0;

    public:
        //interface for select task
        virtual void _on_connect(io_fd* fd) = 0;
        virtual void _on_disconnect() = 0;
    };
}

#endif // BASE_COMM_H
