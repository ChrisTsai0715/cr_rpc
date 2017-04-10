#ifndef INET_SOCKET_COMM_H
#define INET_SOCKET_COMM_H

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
#include <iostream>

#include "common/base_thread.h"
#include "select_task.h"
#include "common/cslock.h"
#include "base_comm.h"
#include "cr_socket/net_socket.h"

namespace cr_rpc
{
    class inet_socket_comm
    {
    public:
        inet_socket_comm()
            :	_net_socket(0)
        {
        }

        virtual ~inet_socket_comm()
        {
            if (_net_socket != 0)
                _net_socket->close();
        }

    protected:
        CRefObj<cr_common::net_socket> _create_socket(const std::string path = "");
        bool _get_addr_port(const std::string& addr, uint16_t& port, std::string& dest_addr)
        {
            size_t pos = addr.find_first_of(":");
            if (pos == std::string::npos)
            {
                std::cerr << "[cr_rpc]inet socket connect not find port" << std::endl;
                return false;
            }

            port = atoi(addr.substr(pos).c_str());
            dest_addr = addr.substr(0, pos);
            return true;
        }

    protected:
        CRefObj<cr_common::net_socket> _net_socket;
    };

    class isocket_comm_client : protected inet_socket_comm,
                               public base_comm_client
    {
    public:
        explicit isocket_comm_client(comm_client_listener* listener)
            :   base_comm_client(listener)
        {

        }

        virtual ~isocket_comm_client(){}

        virtual bool start_connect(const std::string& path, unsigned int timeout_ms);
        virtual bool disconnect_server();

        virtual bool read();
        virtual bool write(const char *buf, size_t size);
    };

    class isocket_comm_server : protected inet_socket_comm,
                                public base_comm_server
    {
    public:
        explicit isocket_comm_server(comm_server_listener* listener)
            :   base_comm_server(listener)
        {

        }

        virtual ~isocket_comm_server();

        virtual bool stop_listen();
        virtual bool accept();
        virtual bool read(int client_fd);
        virtual bool write(int client_fd, const char* buf, size_t size);

        virtual bool start_listen(const std::string& path);
        virtual bool accept_done(int client_fd);
        virtual bool disconnect_client(int client_fd);

    public:
        //impelement comm interface
        virtual void _on_connect(int socket_fd);
        virtual void _on_disconnect_server(int socket_fd);
        virtual void _on_data_receive(int fd, char* buf, size_t size);
        virtual void _on_data_send(int fd, size_t size);

    private:
        std::list<int> _client_sockets;
        cr_common::condition _cond;
    };
}
#endif // UNIX_SOCKET_COMM_H
