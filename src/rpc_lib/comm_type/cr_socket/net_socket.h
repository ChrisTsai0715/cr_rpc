#ifndef NET_SOCKET_H
#define NET_SOCKET_H

#include <sys/socket.h>
#include <sys/types.h>
#include <string>
#include <unistd.h>

#include "IReference.h"
#include "io_fd.h"

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

    class net_socket : public io_fd
    {
    public:
        explicit net_socket(stream_type type)
            : 	_stream_type(type)
        {

        }

        net_socket(io_async_listener* listener, CRefObj<cr_rpc::select_tracker> tracker, stream_type type)
            :	io_fd(listener, tracker),
                _stream_type(type)
        {

        }

        virtual ~net_socket()
        {

        }

        virtual int init() = 0;
        virtual int bind(const std::string& addr) = 0;
        virtual int close()
        {
            if (_fd > 0)
                ::close(_fd);

            return 0;
        }
        int get_sock_addr(sockaddr* addr)
        {
            if (_fd == 0) return -1;

            socklen_t len;
            return getsockname(_fd, addr, &len);
        }

        socket_type get_socket_type() const{return _socket_type;}

    protected:
        stream_type _stream_type;
        socket_type _socket_type;
    };
}

#endif // NET_SOCKET_H
