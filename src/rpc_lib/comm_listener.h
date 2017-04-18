#ifndef COMM_LISTENER_H
#define COMM_LISTENER_H

#include "stdlib.h"
#include "cr_socket/net_socket.h"

namespace cr_common {

    class base_comm;

    class comm_base_listener
    {
    public:
        virtual ~comm_base_listener() {}
        virtual void _on_data_receive(ref_obj<base_comm> fd, char* buf, size_t size) = 0;
        virtual void _on_data_send(ref_obj<base_comm> fd, size_t size) = 0;
    };

    class comm_server_listener : public comm_base_listener
    {
    public:
        virtual ~comm_server_listener() {}
        virtual void _on_client_connect(ref_obj<base_comm> client_fd) = 0;
        virtual void _on_client_disconnect(ref_obj<base_comm> client_fd) = 0;
    };

    class comm_client_listener : public comm_base_listener
    {
    public:
        virtual ~comm_client_listener() {}
        virtual void _on_connect(ref_obj<base_comm> fd) = 0;
        virtual void _on_disconnect_server(ref_obj<base_comm> fd) = 0;
    };
}


#endif // COMM_LISTENER_H
