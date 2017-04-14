#include "inet_socket.h"
#include "net_connect_utils.h"
#include <signal.h>
#include <assert.h>

using namespace cr_common;

inet_socket::inet_socket(stream_type type)
    :	net_socket(type)
{
}

int inet_socket::init()
{
    if ((_fd = ::socket(AF_INET, _stream_type == STREAM_TCP ? SOCK_STREAM : SOCK_DGRAM, 0)) < 0)
    {
        perror("inet socket init error");
        _fd = 0;
        return -1;
    }

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGPIPE);
    sigprocmask(SIG_BLOCK, &set, NULL);

    return _fd;
}

int inet_socket::bind(const std::string &addr)
{
    assert(addr.size());

    int ret = CONNECTUTILS_BIND(_fd, addr.c_str());
    if (ret != NCR_OK) return ret;

    return 0;
}
