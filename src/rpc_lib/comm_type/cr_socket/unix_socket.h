#ifndef UNIX_SOCKET_H
#define UNIX_SOCKET_H

#include <sys/un.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <string>
#include "net_socket.h"

namespace cr_common {
    class unix_socket : public net_socket
    {
    public:
        unix_socket(stream_type type);
        virtual ~unix_socket(){}

        virtual int init();
        virtual int bind(const std::string &addr);
    };

}
#endif // UNIX_SOCKET_H
