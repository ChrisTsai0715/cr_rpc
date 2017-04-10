#ifndef NET_SOCKET_H
#define NET_SOCKET_H

#include <sys/socket.h>
#include <sys/types.h>
#include <string>
#include <unistd.h>

#include "IReference.h"

namespace cr_common{

    typedef int SOCKET;
    typedef enum
    {
        STREAM_UDP,
        STREAM_TCP,
    }stream_type;

    typedef enum
    {
        SOCKET_INET,
        SOCKET_UNIX,
    }socket_type;

    class net_socket : public CReference
    {
    public:
        net_socket(stream_type type)
            : _socket_fd(0),
              _stream_type(type)
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
        virtual int close()
        {
            if (_socket_fd > 0)
                ::close(_socket_fd);

            return 0;
        }
        int get_sock_addr(sockaddr* addr)
        {
            if (_socket_fd == 0)
                return -1;

            socklen_t len;
            return getsockname(_socket_fd, addr, &len);
        }

        operator SOCKET() const{return _socket_fd;}
        socket_type get_socket_type() const{return _socket_type;}

    protected:
        SOCKET _socket_fd;
        stream_type _stream_type;
        socket_type _socket_type;
    };
}

#endif // NET_SOCKET_H
