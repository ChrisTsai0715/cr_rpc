#ifndef SOCKET_CONNECT_H
#define SOCKET_CONNECT_H

#include <string>
#include <netdb.h>
#include "IReference.h"
#include "select_task.h"
#include "comm_listener.h"
#include "net_socket.h"

namespace cr_common{

    class socket_connect
    {
    public:
        socket_connect(cr_rpc::select_tracker &tracker, cr_rpc::comm_base_listener* listener);
        ~socket_connect(){}

        int operator()(CRefObj<net_socket> socket_fd, const std::string& dest, uint16_t port);

    private:
        cr_rpc::select_tracker& _tracker;
        cr_rpc::comm_base_listener* _listener;
    };
}

#endif // SOCKET_CONNECT_H
