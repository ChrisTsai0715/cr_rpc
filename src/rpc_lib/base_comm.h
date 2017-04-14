#ifndef BASE_COMM_H
#define BASE_COMM_H

#include <unistd.h>
#include "common/IReference.h"
#include "select_tracker.h"

namespace cr_rpc
{
    class base_comm : public CReference
    {
    public:
        base_comm();

        virtual ~base_comm();

    protected:
        virtual bool _read(int id);
        virtual bool _write(int id, const char* buf, size_t size);

    public:
        virtual void _on_data_receive(int, char*, ssize_t) = 0;
        virtual void _on_data_send(int, ssize_t) = 0;

    protected:
        select_tracker _select_tracker;
        CMutexLockEx   _read_mutex;
        CMutexLockEx   _write_mutex;
    };

    class base_comm_client : public base_comm
    {
    public:
        base_comm_client()
        {

        }

        virtual ~base_comm_client()
        {

        }

        virtual bool start_connect(const std::string& path, unsigned int timeout_ms) = 0;
        virtual bool disconnect_server() = 0;

        virtual bool read() = 0;
        virtual bool write(const char *buf, size_t size) = 0;

    public:
        //interface for select task
        virtual void _on_connect(int client_fd) = 0;
        virtual void _on_disconnect(int client_fd) = 0;
    };

    class base_comm_server : public base_comm
    {
    public:
        base_comm_server()
        {

        }

        virtual ~base_comm_server()
        {

        }

        virtual bool start_listen(const std::string&) = 0;
        virtual bool stop_listen() = 0;

        virtual bool accept() = 0;
        virtual bool read(int client_fd) = 0;
        virtual bool write(int client_fd, const char*, size_t) = 0;
        virtual bool disconnect_client(int client_fd) = 0;

    public:
        //interface for select task
        virtual void _on_connect(int socket_fd) = 0;
        virtual void _on_disconnect_server(int socket_fd) = 0;
    };
}

#endif // BASE_COMM_H
