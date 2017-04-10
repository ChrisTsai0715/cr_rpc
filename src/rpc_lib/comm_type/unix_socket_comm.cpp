#include "unix_socket_comm.h"
#include <signal.h>

using namespace cr_rpc;

int unet_socket_comm::_create_socket(const std::string &path)
{
    assert(path.size());

    int fd;
    struct sockaddr_un addr;
    bzero((void*)&addr, sizeof(addr));

    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
        perror("socket create failed...");
        throw std::runtime_error("socket create failed");
    }

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGPIPE);
    sigprocmask(SIG_BLOCK, &set, NULL);

    return fd;
}

bool usocket_comm_client::start_connect(const std::string &path, unsigned int timeout_ms)
{
    _socket_fd = _create_socket(path);

    struct sockaddr_un server_addr;
    bzero((void *)&server_addr, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, path.c_str());

    fcntl(_socket_fd, F_SETFL, O_NONBLOCK);

    int connect_err;
    if (0 == (connect_err = connect(_socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr))))
        return true;
    else if (errno != EINPROGRESS)
    {
        perror("connect error");
        throw std::runtime_error("connect server failed");;
    }

    fd_set connect_sockets;
    FD_ZERO(&connect_sockets);
    FD_SET(_socket_fd, &connect_sockets);
    struct timeval connect_timeout;
    connect_timeout.tv_sec = timeout_ms / 1000;
    connect_timeout.tv_usec = (timeout_ms % 1000) * 1000;

    int select_ret = 0;
    if ((select_ret = select(_socket_fd + 1, NULL, &connect_sockets, NULL, &connect_timeout)) > 0)
    {
        if (FD_ISSET(_socket_fd, &connect_sockets) == 0)
            throw std::runtime_error("connect server timeout");

        int err;
        socklen_t err_len = sizeof(err);
        getsockopt(_socket_fd, SOL_SOCKET, SO_ERROR, &err, &err_len);
        if (err != 0)
            throw std::runtime_error("connect server failed..");
    }
    else
    {
        throw std::runtime_error("connect server timeout");
    }

    return true;
}

bool usocket_comm_client::disconnect_server()
{
    if (_socket_fd != -1)
    {
        _select_tracker.del_task(_socket_fd, select_task::TASK_SELECT_READ);
        _select_tracker.del_task(_socket_fd, select_task::TASK_SELECT_WRITE);
        _socket_fd = -1;
        return shutdown(_socket_fd, SHUT_RDWR) == 0;
    }

    return false;
}

bool usocket_comm_client::read()
{
    return _read(_socket_fd);
}

bool usocket_comm_client::write(const char *buf, size_t size)
{
    return _write(_socket_fd, buf, size);
}

usocket_comm_server::~usocket_comm_server()
{
    stop_listen();
 //   disconnect_client();
}

bool usocket_comm_server::start_listen(const std::string &path)
{
    _socket_fd = _create_socket(path);

    sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, path.c_str());

    int bind_size;
    bind_size = offsetof(sockaddr_un, sun_path) + strlen(addr.sun_path);
    unlink (addr.sun_path);
    if (bind(_socket_fd, (struct sockaddr *)&addr, bind_size) < 0)
    {
        perror("socket bind failed..");
        throw std::runtime_error("socket bind failed");
    }

    if (listen(_socket_fd, 5) < 0)
    {
        perror("socket bind failed...");
        throw std::runtime_error("socket listen failed");
    }

    fcntl(_socket_fd, F_SETFL, O_NONBLOCK);
    this->accept();

    return true;
}

bool usocket_comm_server::accept_done(int client_fd)
{
    if (client_fd != -1)
    {
//        if (listener != NULL) listener->_client_disconnect(_client_fd);
        _select_tracker.del_task(client_fd, select_task::TASK_SELECT_READ);
        _select_tracker.del_task(client_fd, select_task::TASK_SELECT_WRITE);
        shutdown(client_fd, SHUT_RDWR);
    }
//    if (_listener) _listener->_client_connect(_client_fd);
}

bool usocket_comm_server::stop_listen()
{
    if (_socket_fd != -1)
    {
        _select_tracker.del_task(_socket_fd, select_task::TASK_SELECT_ACCEPT);
        return shutdown(_socket_fd, SHUT_RDWR) == 0;
    }
    return true;
}

bool usocket_comm_server::disconnect_client(int client_fd)
{
    if (client_fd != -1)
    {
        _select_tracker.del_task(client_fd, select_task::TASK_SELECT_READ);
        _select_tracker.del_task(client_fd, select_task::TASK_SELECT_WRITE);
        return shutdown(client_fd, SHUT_RDWR) == 0;
    }
    return true;
}

bool usocket_comm_server::accept()
{
    return _select_tracker.add_task(
                    socket_accept_task::new_instance(_socket_fd, _listener)
            );
}

bool usocket_comm_server::read(int client_fd)
{
    return _read(client_fd);
}

bool usocket_comm_server::write(int client_fd, const char *buf, size_t size)
{
    return _write(client_fd, buf, size);
}
