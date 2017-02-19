#include "base_rpc_server.h"

using namespace cr_rpc;
base_rpc_server::base_rpc_server(rpc_comm_type_def type)
{
    switch(type)
    {
    default:
    case RPC_COMM_TYPE_SOCKET:
        _unix_comm_server = new socket_comm_server(this);
        break;

    case RPC_COMM_TYPE_FIFO:
        _unix_comm_server = new fifo_comm_server(this);
        break;
    }
}

base_rpc_server::~base_rpc_server()
{

}

bool base_rpc_server::start_listen()
{
    try
    {
        _unix_comm_server->start_listen(_socket_path);
        _rpc_handler.run();
    }
    catch(std::runtime_error& e)
    {
        printf("listen err:%s\n", e.what());
        return false;
    }

    return true;
}

bool base_rpc_server::send_req(const std::string &cmd, rpc_req_args_type &req_map)
{
    std::string json_str;
    _format_req_msg(cmd, req_map, json_str);

    return _unix_comm_server->write(json_str.c_str(), json_str.size());
}

void base_rpc_server::_client_connect(int fd)
{
    fd = fd;
    _unix_comm_server->accept();
    _unix_comm_server->read();
}

void base_rpc_server::_client_disconnect(int fd)
{
    fd = fd;
    _unix_comm_server->disconnect_client();
}

void base_rpc_server::_client_data_receive(int fd, char *buf, size_t size)
{
    fd = fd;
    _receive_data_handle(buf, size);
    _unix_comm_server->read();
}

void base_rpc_server::_client_data_send(int fd, size_t size)
{
    fd = fd;
    size = size;
}
