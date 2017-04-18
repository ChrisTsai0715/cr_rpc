#include "base_rpc_client.h"
#include "cr_socket/inet_socket.h"

using namespace cr_common;
rpc_client::rpc_client(rpc_comm_type_def type)
{
    _tracker = new cr_common::select_tracker;

    switch(type)
    {
    default:
    case RPC_COMM_TYPE_INET:
        _unix_comm_client = new inet_client(STREAM_TCP, _tracker, this);
        break;

    case RPC_COMM_TYPE_FIFO:
        //_unix_comm_client = new fifo_comm_client(this);
        break;
    }
}

rpc_client::~rpc_client()
{

}

bool rpc_client::start_connect()
{
    try
    {
        _unix_comm_client->start_connect(_socket_path, 3000);
        _unix_comm_client->recv_data(NULL, 2048);
        _rpc_handler.run();
    }
    catch(std::runtime_error& e)
    {
        printf("connect err:%s\n", e.what());
        return false;
    }

    return true;
}

bool rpc_client::send_req(const std::string &cmd, rpc_req_args_type &req_map)
{
    std::string json_str;
    _format_req_msg(cmd, req_map, json_str);

    return _unix_comm_client->send_data(json_str.c_str(), json_str.size());
}

void rpc_client::_on_disconnect_server(ref_obj<base_comm> fd)
{

}

void rpc_client::_on_connect(ref_obj<base_comm> socket_fd)
{

}

void rpc_client::_on_data_receive(ref_obj<base_comm> fd, char *buf, size_t size)
{
    fd = fd;
    _receive_data_handle(buf, size);
    _unix_comm_client->recv_data(NULL, 0);
}

void rpc_client::_on_data_send(ref_obj<base_comm> fd, size_t size)
{
    size = size;
}
