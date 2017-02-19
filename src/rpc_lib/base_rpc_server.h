#ifndef BASE_RPC_SERVER_H
#define BASE_RPC_SERVER_H

#include "base_rpc_interface.h"

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
    class base_rpc_server : protected base_rpc_interface,
                            public comm_server_listener
    {
    public:
        explicit base_rpc_server(rpc_comm_type_def type = RPC_COMM_TYPE_SOCKET);
        virtual ~base_rpc_server();
        bool start_listen();

    public:
        virtual bool send_req(const std::string& cmd, rpc_req_args_type& req_map);

    public:
        virtual void _client_connect(int fd);

        virtual void _client_disconnect(int fd);

        virtual void _client_data_receive(int fd, char* buf, size_t size);

        virtual void _client_data_send(int fd, size_t size);

    private:
        CRefObj<base_comm_server> _unix_comm_server;
    };
}
#endif // BASE_RPC_SERVER_H
