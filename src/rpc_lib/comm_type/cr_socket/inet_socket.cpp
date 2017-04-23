#include "inet_socket.h"
#include <signal.h>
#include <assert.h>
#include <stdlib.h>
#include <iostream>
#include <netdb.h>
#include "net_connect_utils.h"
#include "select_task.h"

using namespace cr_common;

int inet_socket::init()
{
    int fd = 0;
    if ((fd = ::socket(AF_INET,
                       _stream_type == STREAM_TCP ? SOCK_STREAM : SOCK_DGRAM,
                       0)) < 0)
    {
        perror("inet socket init error");
        fd = 0;
        return -1;
    }

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGPIPE);
    sigprocmask(SIG_BLOCK, &set, NULL);

    //set socket async
    if (_listener != 0)	fcntl(_fd, F_SETFL, O_NONBLOCK);
    return fd;
}

bool inet_socket::delete_all_task()
{
    if (_tracker == 0) return true;
    return _tracker->del_task(_fd);
}

ssize_t inet_socket::recv_data(char *buf, size_t size)
{
    if (_tracker != 0) return net_socket::async_read();
    return net_socket::read(buf, size);
}

ssize_t inet_socket::send_data(const char *buf, size_t size)
{
    if (_tracker != 0) return net_socket::async_write(buf, size);
    return net_socket::write(buf, size);
}

void inet_socket::on_read_done(char *buf, ssize_t size)
{
    this->_on_data_receive(buf, size);
}

void inet_socket::on_write_done(ssize_t size)
{
    this->_on_data_send(size);
}

int inet_socket::_bind(const std::string &addr)
{
    assert(addr.size());

    int ret = CONNECTUTILS_BIND(_fd, addr.c_str());
    if (ret != NCR_OK) return ret;

    return 0;
}

bool inet_socket::_acquire_domain_port(const std::string &addr, std::string &domain, uint16_t &port)
{
    size_t pos = addr.find_first_of(":");
    if (pos == std::string::npos) return false;

    port = atoi(addr.substr(pos + 1).c_str());
    domain = addr.substr(0, pos);
    return true;
}

void inet_socket::_on_data_receive(char *buf, ssize_t size)
{
    if (_listener)
    {
        size > 0 ?
        _listener->_on_data_receive(this, buf, size)
            :
        _on_disconnect();
    }
}

void inet_socket::_on_data_send(ssize_t size)
{
    if (_listener) _listener->_on_data_send(this, size);
}

void inet_socket::_on_disconnect()
{
    //if socket fd disconnect then delete all tracker task.
    delete_all_task();
    this->close();

    if (_listener != NULL)
        _listener->_on_disconnect(this);
}

int inet_socket::disconnect()
{
    if (_fd > 0) ::close(_fd);
    return 0;
}


/*********************
 * inet_server
 ********************/
int inet_server::start_listen(const std::string &path)
{
    _fd = init();
    if (_fd <= 0) return -1;
    if (_bind(path) != 0) return -2;

    if (listen(_fd, 5) < 0)
    {
        perror("socket bind failed...");
        return -3;
    }

    accept();

    return true;

}

int inet_server::stop_listen()
{
    if (_fd != 0)
    {
        _tracker->del_task(_fd, select_task::TASK_SELECT_ACCEPT);
        return shutdown(_fd, SHUT_RDWR) == 0;
    }
    return true;
}

int inet_server::accept()
{
    return _tracker->add_task(socket_accept_task::new_instance(_fd, this, _tracker)) ? 0 : -1;
}

void inet_server::_on_data_receive(char *buf, ssize_t size)
{
    inet_socket::_on_data_receive(buf, size);
}

void inet_server::_on_data_send(ssize_t size)
{
    inet_socket::_on_data_send(size);
}

void inet_server::_on_connect(ref_obj<base_comm> client)
{
    if (_listener) _listener->_on_client_connect(client);
}

ssize_t inet_server::recv_data(char *buf, size_t size)
{
    return inet_socket::recv_data(buf, size);
}

ssize_t inet_server::send_data(const char *buf, size_t size)
{
    return inet_socket::send_data(buf, size);
}

bool inet_server::socket_accept_task::done()
{
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client_fd = ::accept(_socket_fd, (struct sockaddr *)&client_addr, &addr_len);
    if (client_fd < 0)
    {
        ::perror("accept err");
        return false;
    }

    base_comm_server* listener = _get_listener<base_comm_server>();
    if (listener != NULL) listener->_on_connect(new inet_socket(client_fd, _tracker, listener->get_comm_listener()));
    return true;
}


/***********************
 * inet_client
 ***********************/
int inet_client::start_connect(const std::string &path, bool block, unsigned int timeout_ms)
{
    std::string domain;
    uint16_t port;
    if (!_acquire_domain_port(path, domain, port))
        throw std::runtime_error("get domain and port failed");

    _fd = init();
    if (_fd <= 0) throw std::runtime_error("client get new socket fd err");

    if (block)
        return CONNECTUTILS_CONNECT(_fd, domain.c_str(), port, (char *)0, timeout_ms);

    return _tracker->add_task(
                socket_connect_task::new_instance(_fd, this, domain,port)
                );

}

int inet_client::disconnect_server()
{
    return net_socket::close();
}

ssize_t inet_client::recv_data(char *buf, size_t size)
{
    return inet_socket::recv_data(buf, size);
}

void inet_client::_on_data_receive(char *buf, ssize_t size)
{
    size > 0 ?
        inet_socket::_on_data_receive(buf, size)
            :
        _on_disconnect();
}

void inet_client::_on_data_send(ssize_t size)
{
    inet_socket::_on_data_send(size);
}

void inet_client::_on_connect()
{
    if (_listener != NULL)
        _listener->_on_connect(this);
}

ssize_t inet_client::send_data(const char *buf, size_t size)
{
    return inet_socket::send_data(buf, size);
}

bool inet_client::socket_connect_task::done()
{
    struct hostent *host = gethostbyname(_dest_addr.c_str());
    if (host == NULL)
    {
        herror("get host by name");
        return false;
    }
    char* host_addr = host->h_addr_list[0];

    struct sockaddr_in connect_addr;
    connect_addr.sin_family = AF_INET;
    connect_addr.sin_port = htons(_port);
    connect_addr.sin_addr.s_addr = *((in_addr_t*)host_addr);
    socklen_t len = sizeof(struct sockaddr_in);
    int ret = ::connect(_socket_fd, (struct sockaddr *)&connect_addr, len);
    if (ret < 0)
    {
        ::perror("connect err");
        return false;
    }

   base_comm_client* listener = _get_listener<base_comm_client>();
   if (listener != NULL) listener->_on_connect();

   return true;
}
