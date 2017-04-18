#ifndef BASE_RPC_CLIENT_H
#define BASE_RPC_CLIENT_H

#include "base_rpc_interface.h"
//#include "comm_type/inet_socket_comm.h"
//#include "fifo_comm.h"
#include "common/IReference.h"

namespace cr_common
{
    /*******************
     *
     * base rpc client
     *
     * usage : Should specify the type of base comm, as RPC_COMM_TYPE_SOCKET
     * 		   or RPC_COMM_TYPE_FIFO
     * 		   You can declare a class to inherit base_rpc_client
     *
     * ****************/

    class rpc_client : protected base_rpc,
                       public comm_client_listener
    {
    public:
        explicit rpc_client(rpc_comm_type_def type = RPC_COMM_TYPE_INET);
        virtual ~rpc_client();
        bool start_connect();

    public:
        virtual bool send_req(const std::string& cmd, rpc_req_args_type& req_map);

    public:
        virtual void _on_disconnect_server(ref_obj<base_comm> fd);
        virtual void _on_connect(ref_obj<base_comm> socket_fd);
        virtual void _on_data_receive(ref_obj<base_comm> fd, char* buf, size_t size);
        virtual void _on_data_send(ref_obj<base_comm> fd, size_t size);

    private:
        ref_obj<base_comm_client> _unix_comm_client;
        ref_obj<cr_common::select_tracker> _tracker;
    };
}
#endif // BASE_RPC_CLIENT_H
