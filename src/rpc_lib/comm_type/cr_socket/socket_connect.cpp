#include "socket_connect.h"
#include <fcntl.h>
#include <stdexcept>

using namespace cr_common;

socket_connect::socket_connect(cr_rpc::select_tracker& tracker, cr_rpc::base_comm *listener)
    :	_tracker(tracker),
        _listener(listener)
{
}

int socket_connect::operator()(CRefObj<net_socket> socket_fd, const std::string &dest, uint16_t port)
{
   if (socket_fd->get_socket_type() == SOCKET_INET)
   {
        struct hostent *host = gethostbyname(dest.c_str());
        if (host == NULL)
        {
            herror("get host by name");
            return -2;
        }

        char* host_addr = host->h_addr_list[0];

        struct sockaddr_in dest_addr;
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(port);
        dest_addr.sin_addr.s_addr = *((in_addr_t*)host_addr);
    #warning
        _tracker.add_task(cr_rpc::socket_connect_task::new_instance(*socket_fd, _listener, 123, port));
   }
   else if (socket_fd->get_socket_type() == SOCKET_UNIX)
   {

        struct sockaddr_un server_addr;
        bzero((void *)&server_addr, sizeof(server_addr));
        server_addr.sun_family = AF_UNIX;
        strcpy(server_addr.sun_path, dest.c_str());

        fcntl(*socket_fd, F_SETFL, O_NONBLOCK);

        int connect_err;
        if (0 == (connect_err = connect(*socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr))))
            return true;
        else if (errno != EINPROGRESS)
        {
            perror("connect error");
            throw std::runtime_error("connect server failed");;
        }

        _tracker.add_task(cr_rpc::socket_connect_task::new_instance(*socket_fd, _listener, 123, port));
   }

    return 0;
}

