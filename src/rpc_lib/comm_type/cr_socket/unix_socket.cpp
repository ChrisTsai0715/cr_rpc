#include "unix_socket.h"
#include <signal.h>
#include <assert.h>

using namespace cr_common;

unix_socket::unix_socket(socket_type type)
    :	net_socket(type)
{

}

int unix_socket::init()
{
    if((_socket_fd = ::socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
        perror("unix socket init error");
        _socket_fd = 0;
        return -1;
    }

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGPIPE);
    sigprocmask(SIG_BLOCK, &set, NULL);

    return _socket_fd;
}

int unix_socket::bind(const std::string &addr)
{
    assert(addr.size());
    if (_socket_fd == 0)
        return -1;

    sockaddr_un sockaddr;
    int bind_size = offsetof(sockaddr_un, sun_path) + strlen(sockaddr.sun_path);
    unlink(sockaddr.sun_path);
    addr.sun_family = AF_UNIX;
    strcpy(sockaddr.sun_path, addr.c_str());
    if(::bind(_socket_fd, (struct sockaddr *)&sockaddr, bind_size) < 0)
    {
        perror("unix socket bind failed");
        return -2;
    }

    return 0;
}
