#include "inet_socket.h"
#include "net_connect_utils.h"
#include <signal.h>
#include <assert.h>

using namespace cr_common;

inet_socket::inet_socket(socket_type type)
    :	_socket_type(type)
{
}

int inet_socket::init()
{
    if ((_socket_fd = ::socket(AF_INET, _socket_type == SOCKET_TCP ? SOCK_STREAM : SOCK_DGRAM, 0)) < 0)
    {
        perror("inet socket init error");
        _socket_fd = 0;
        return -1;
    }

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGPIPE);
    sigprocmask(SIG_BLOCK, &set, NULL);

    return _socket_fd;
}

int inet_socket::bind(const std::string &addr)
{
    assert(addr.size());

    int ret = NETCONNECTUTILS(addr);
    if (ret != NCR_OK)
        return ret;

    return 0;
}