#ifndef BASE_RPC_SERVER_H
#define BASE_RPC_SERVER_H

#include "select_tracker.h"
#include "cr_socket/net_socket.h"
#include "base_rpc_interface.h"
//#include "inet_socket_comm.h"
//#include "unix_socket_comm.h"
//#include "fifo_comm.h"

namespace cr_common
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
        virtual void _on_data_receive(io_fd* fd, char* buf, size_t size);
        virtual void _on_data_send(io_fd* fd, size_t size);
        virtual void _on_client_connect(io_fd* client_fd);
        virtual void _on_client_disconnect(io_fd* client_fd);

    private:
        virtual bool _write(const char *buf, size_t size);
        virtual bool _read();

    private:
        CRefObj<base_comm_server> _comm_server;
        std::list<int> _client_fd_lists;
        CRefObj<cr_common::select_tracker> _tracker;
    };
}
#endif // BASE_RPC_SERVER_H
