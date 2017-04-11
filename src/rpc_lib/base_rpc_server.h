#ifndef BASE_RPC_SERVER_H
#define BASE_RPC_SERVER_H

#include "base_rpc_interface.h"
#include "inet_socket_comm.h"
//#include "unix_socket_comm.h"
//#include "fifo_comm.h"

namespace cr_rpc
{
    /*******************
     *
     * base rpc server
     *
     * usage : Should specify the type of base comm, as RPC_COMM_TYPE_SOCKET
     * 		   or RPC_COMM_TYPE_FIFO
     * 		   You can declare a class to inherit base_rpc_server
     *
     * ****************/
    class rpc_server : public base_rpc,
                       public comm_server_listener
    {
    public:
        explicit rpc_server(rpc_comm_type_def type = RPC_COMM_TYPE_INET);
        virtual ~rpc_server();
        bool start_listen();

    public:
        virtual bool send_req(const std::string& cmd, rpc_req_args_type& req_map);

    public:
        virtual void _data_receive(int fd, char* buf, size_t size);
        virtual void _data_send(int fd, size_t size);
        virtual void _client_connect(int client_fd);
        virtual void _client_disconnect(int client_fd);

    private:
        CRefObj<base_comm_server> _comm_server;
        std::list<int> _client_fd_lists;
    };
}
#endif // BASE_RPC_SERVER_H
