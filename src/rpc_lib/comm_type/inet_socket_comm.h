#ifndef UNIX_SOCKET_COMM_H
#define UNIX_SOCKET_COMM_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string>
#include <stdexcept>
#include <fcntl.h>
#include <stddef.h>
#include <list>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>

#include "common/base_thread.h"
#include "select_task.h"
#include "common/cslock.h"
#include "base_comm.h"

namespace cr_rpc
{
    class unix_socket_comm
    {
    public:
        unix_socket_comm()
         :   _socket_fd(-1)
        {
        }

        virtual ~unix_socket_comm()
        {
            if (_socket_fd != -1)
                close(_socket_fd);
        }

    protected:
        int _create_socket(const std::string& path);

    protected:
        int _socket_fd;
    };

    class socket_comm_client : protected unix_socket_comm,
                               public base_comm_client
    {
    public:
        explicit socket_comm_client(comm_client_listener* listener)
            :   base_comm_client(listener)
        {

        }

        virtual ~socket_comm_client(){}

        virtual bool start_connect(const std::string& path, unsigned int timeout_ms);
        virtual bool disconnect_server();

        virtual bool read();
        virtual bool write(const char *buf, size_t size);
    };

    class socket_comm_server : protected unix_socket_comm,
                               public base_comm_server
    {
    public:
        explicit socket_comm_server(comm_server_listener* listener)
            :   base_comm_server(listener),
                _client_fd(-1)
        {

        }

        virtual ~socket_comm_server();

        virtual bool accept();
        virtual bool read();
        virtual bool write(const char* buf, size_t size);

        virtual bool start_listen(const std::string& path);
        virtual bool accept_done(int client_fd);
        virtual bool stop_listen();
        virtual bool disconnect_client();

    private:
        int _client_fd;
    };
}
#endif // UNIX_SOCKET_COMM_H
