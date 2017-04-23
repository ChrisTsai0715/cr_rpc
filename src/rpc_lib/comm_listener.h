#ifndef COMM_LISTENER_H
#define COMM_LISTENER_H

#include "stdlib.h"
#include "cr_socket/net_socket.h"

namespace cr_common {

    class base_comm;

    class comm_listener
    {
    public:
        virtual ~comm_listener() {}
        //interfaces for base comm
        virtual void _on_data_receive(ref_obj<base_comm> fd, char* buf, size_t size) = 0;
        virtual void _on_data_send(ref_obj<base_comm> fd, size_t size) = 0;
        virtual void _on_disconnect(ref_obj<base_comm> fd) = 0;
        //interfaces for comm server
        virtual void _on_client_connect(ref_obj<base_comm> client_fd ){}
        //interfaces for comm client
        virtual void _on_connect(ref_obj<base_comm> fd) {}
    };
}


#endif // COMM_LISTENER_H
