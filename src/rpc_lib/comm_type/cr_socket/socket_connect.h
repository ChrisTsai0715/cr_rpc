#ifndef SOCKET_CONNECT_H
#define SOCKET_CONNECT_H

#include <string>
#include <netdb.h>
#include "IReference.h"
#include "select_task.h"
#include "comm_listener.h"
#include "net_socket.h"
#include "select_tracker.h"
#include "base_comm.h"

namespace cr_common{

    class socket_connect
    {
    public:
        socket_connect(cr_common::select_tracker &tracker, cr_common::base_comm* listener);
        ~socket_connect(){}

        int operator()(ref_obj<net_socket> socket_fd, const std::string& dest, uint16_t port);

    private:
        cr_common::select_tracker& _tracker;
        cr_common::base_comm* _listener;
    };
}

#endif // SOCKET_CONNECT_H
