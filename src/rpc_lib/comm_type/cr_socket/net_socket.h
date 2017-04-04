#ifndef NET_SOCKET_H
#define NET_SOCKET_H

#include <sys/socket.h>
#include <sys/types.h>
#include <string>

namespace cr_common{

    typedef int SOCKET;
    typedef enum
    {
        SOCKET_UDP,
        SOCKET_TCP,
    }socket_type;

    class net_socket
    {
    public:
        net_socket(socket_type type)
            : _socket_fd(0),
              _socket_type(type)
        {
        }

        virtual ~net_socket()
        {
            if (_socket_fd > 0)
            {
                ::close(_socket_fd);
                _socket_fd = 0;
            }
        }

        virtual int init() = 0;
        virtual int bind(const std::string& addr) = 0;
        int get_sock_addr(sockaddr* addr)
        {
            if (_socket_fd == 0)
                return -1;

            socklen_t len;
            return getsockname(_socket_fd, addr, &len);
        }

        operator SOCKET() const{return _socket_fd;}

    protected:
        SOCKET _socket_fd;
        socket_type _socket_type;
    };
}

#endif // NET_SOCKET_H
