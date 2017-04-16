#include "base_rpc_server.h"
#include <algorithm>
#include "cr_socket/inet_socket.h"

using namespace cr_common;

rpc_server::rpc_server(rpc_comm_type_def type)
{
    _tracker = new cr_common::select_tracker;

    switch(type)
    {
    default:
    case RPC_COMM_TYPE_INET:
        _comm_server = new inet_server(STREAM_TCP, _tracker, this);
        break;
    case RPC_COMM_TYPE_UNIX:
 //       _comm_server = new usocket_comm_server(this);

    case RPC_COMM_TYPE_FIFO:
  //      _comm_server = new fifo_comm_server(this);
        break;
    }
}

rpc_server::~rpc_server()
{

}

bool rpc_server::start_listen()
{
    try
    {
        _comm_server->start_listen(_socket_path);
        _rpc_handler.run();
    }
    catch(std::runtime_error& e)
    {
        printf("listen err:%s\n", e.what());
        return false;
    }

    return true;
}

bool rpc_server::send_req(const std::string &cmd, rpc_req_args_type &req_map)
{
    std::string json_str;
    _format_req_msg(cmd, req_map, json_str);

    return _write(json_str.c_str(), json_str.size());
}

void rpc_server::_on_data_receive(io_fd *fd, char *buf, size_t size)
{
    fd = fd;
    _receive_data_handle(buf, size);
    _read();
}

void rpc_server::_on_data_send(io_fd* fd, size_t size)
{
    fd = fd;
    size = size;
}

void rpc_server::_on_client_connect(io_fd *client_fd)
{
    if (std::find(_client_fd_lists.begin(), _client_fd_lists.end(), client_fd) == _client_fd_lists.end())
        _client_fd_lists.push_back(client_fd);
    _comm_server->accept();
}

void rpc_server::_on_client_disconnect(io_fd* client_fd)
{
    if (std::find(_client_fd_lists.begin(), _client_fd_lists.end(), client_fd) != _client_fd_lists.end())
        _comm_server->disconnect_client(client_fd);
}

bool rpc_server::_write(const char *buf, size_t size)
{
    return _comm_server->write(_client_fd_lists.front(), buf, size);
}

bool rpc_server::_read()
{
    return _comm_server->read(_client_fd_lists.front());
}
