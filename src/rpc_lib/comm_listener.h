#ifndef COMM_LISTENER_H
#define COMM_LISTENER_H

#include "stdlib.h"
#include "cr_socket/net_socket.h"

namespace cr_common {

    class comm_base_listener
    {
    public:
        virtual ~comm_base_listener() {}
        virtual void _on_data_receive(io_fd* fd, char* buf, size_t size) = 0;
        virtual void _on_data_send(io_fd* fd, size_t size) = 0;
    };

    class comm_server_listener : public comm_base_listener
    {
    public:
        virtual ~comm_server_listener() {}
        virtual void _on_client_connect(io_fd* client_fd) = 0;
        virtual void _on_client_disconnect(io_fd* client_fd) = 0;
    };

    class comm_client_listener : public comm_base_listener
    {
    public:
        virtual ~comm_client_listener() {}
        virtual void _on_connect(io_fd* fd) = 0;
        virtual void _on_disconnect_server(io_fd* fd) = 0;
    };
}


#endif // COMM_LISTENER_H
