#ifndef INET_SOCKET_H
#define INET_SOCKET_H

#include "net_socket.h"

namespace cr_common {

    class inet_socket : public net_socket
    {
    public:
        inet_socket(stream_type type);
        virtual ~inet_socket(){}

    public:
        virtual int init();
        virtual int bind(const std::string& addr);
    };
}
#endif // INET_SOCKET_H
