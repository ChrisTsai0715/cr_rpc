#include "inet_socket_comm.h"
#include <signal.h>
#include <algorithm>
#include "cr_socket/inet_socket.h"
#include "cr_socket/socket_connect.h"

using namespace cr_common;

CRefObj<cr_common::net_socket> inet_socket_comm::_create_socket(const std::string path)
{
    CRefObj<cr_common::net_socket> isocket = new cr_common::inet_socket(cr_common::STREAM_TCP);
    if(path.size() > 0) isocket->_bind(path);

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGPIPE);
    sigprocmask(SIG_BLOCK, &set, NULL);

    return isocket;
}

bool isocket_comm_client::start_connect(const std::string &path, unsigned int timeout_ms)
{
    _net_socket = _create_socket();

    uint16_t port;
    std::string dest_addr;
    if (!_get_addr_port(path, port, dest_addr))
        return false;

    cr_common::socket_connect connector(_select_tracker, this);
    return connector(_net_socket, dest_addr, port) == 0;
}

bool isocket_comm_client::disconnect_server()
{
    if (_net_socket != 0)
    {
        _select_tracker.del_task(*_net_socket, select_task::TASK_SELECT_READ);
        _select_tracker.del_task(*_net_socket, select_task::TASK_SELECT_WRITE);
        _net_socket = 0;
        return shutdown(*_net_socket, SHUT_RDWR) == 0;
    }

    return false;
}

bool isocket_comm_client::read()
{
    return _read(*_net_socket);
}

bool isocket_comm_client::write(const char *buf, size_t size)
{
    return _write(*_net_socket, buf, size);
}

void isocket_comm_client::_on_connect(int client_fd)
{

}

void isocket_comm_client::_on_disconnect(int client_fd)
{

}

void isocket_comm_client::_on_data_receive(int, char *, ssize_t)
{

}

void isocket_comm_client::_on_data_send(int, ssize_t)
{

}

isocket_comm_server::~isocket_comm_server()
{
    stop_listen();
  //  disconnect_client();
}

bool isocket_comm_server::start_listen(const std::string &path)
{
    _net_socket = _create_socket(path);

    if (listen(*_net_socket, 5) < 0)
    {
        perror("socket bind failed...");
        throw std::runtime_error("socket listen failed");
    }

    this->accept();

    return true;
}

bool isocket_comm_server::accept_done(int client_fd)
{
    if (client_fd != -1)
    {
//        if (_listener) _listener->_client_disconnect(_client_fd);
        _select_tracker.del_task(client_fd, select_task::TASK_SELECT_READ);
        _select_tracker.del_task(client_fd, select_task::TASK_SELECT_WRITE);
        shutdown(client_fd, SHUT_RDWR);
    }
//    if (_listener) _listener->_client_connect(_client_fd);
}

bool isocket_comm_server::stop_listen()
{
    if (_net_socket != 0)
    {
        _select_tracker.del_task(*_net_socket, select_task::TASK_SELECT_ACCEPT);
        return shutdown(*_net_socket, SHUT_RDWR) == 0;
    }
    return true;
}

bool isocket_comm_server::disconnect_client(int client_fd)
{
    if (client_fd != -1)
    {
        _select_tracker.del_task(client_fd, select_task::TASK_SELECT_READ);
        _select_tracker.del_task(client_fd, select_task::TASK_SELECT_WRITE);
        return shutdown(client_fd, SHUT_RDWR) == 0;
    }
    return true;
}

void isocket_comm_server::_on_connect(int socket_fd)
{
    cr_common::auto_cond cscond(_cond);
    if (std::find(_client_sockets.begin(), _client_sockets.end(), socket_fd) != _client_sockets.end())
        _client_sockets.push_back(socket_fd);
}

void isocket_comm_server::_on_disconnect_server(int socket_fd)
{
    cr_common::auto_cond cscond(_cond);
    if (std::find(_client_sockets.begin(), _client_sockets.end(), socket_fd) != _client_sockets.end())
        _client_sockets.remove(socket_fd);
}

void isocket_comm_server::_on_data_receive(int fd, char *buf, ssize_t size)
{

}

void isocket_comm_server::_on_data_send(int fd, ssize_t size)
{

}

bool isocket_comm_server::accept()
{
    fcntl(*_net_socket, F_SETFL, O_NONBLOCK);
    return true;//_select_tracker.add_task(
                 //   socket_accept_task::new_instance(*_net_socket, this)
           // );
}

bool isocket_comm_server::read(int client_fd)
{
    return _read(client_fd);
}

bool isocket_comm_server::write(int client_fd, const char *buf, size_t size)
{
    return _write(client_fd, buf, size);
}
