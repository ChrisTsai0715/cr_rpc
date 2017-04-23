#include "base_rpc_client.h"
#include <iostream>
#include "cr_socket/inet_socket.h"

using namespace cr_common;
rpc_client::rpc_client(rpc_comm_type_def type)
    :	_connect_status(false)
{
    _tracker = new cr_common::select_tracker;
    _tracker->run();

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
    if (_connect_status)
    {
        _unix_comm_client->disconnect();
        _connect_status = false;
    }

    try
    {
        _unix_comm_client->start_connect(_socket_path, true, 3000);
        _connect_status = true;
        //start handle received message in rpc handler thread.
        _rpc_handler.run();
        _unix_comm_client->recv_data(NULL, 0);
    }
    catch(const std::runtime_error& e)
    {
        std::cerr << "connect err:" << e.what() << std::endl;
        return false;
    }

    return true;
}

bool rpc_client::send_req(const std::string &cmd, rpc_req_args_type &req_map)
{
    std::string json_str;
    //format request message as some protocol(json or etc.).
    _format_req_msg(cmd, req_map, json_str);

    return _unix_comm_client->send_data(json_str.c_str(), json_str.size());
}

void rpc_client::_on_disconnect(ref_obj<base_comm> fd)
{
    fd = fd;
 //   this->start_connect();
}

void rpc_client::_on_connect(ref_obj<base_comm> fd)
{
    fd = fd;
    //connect success then receive data async.
    _unix_comm_client->recv_data(NULL, 0);
}

void rpc_client::_on_data_receive(ref_obj<base_comm> fd, char *buf, size_t size)
{
    fd = fd;
    //handle the received data.
    _receive_data_handle(buf, size);
    //continue receive data.
    _unix_comm_client->recv_data(NULL, 0);
}

void rpc_client::_on_data_send(ref_obj<base_comm> fd, size_t size)
{
    fd = fd;
    size = size;
}
