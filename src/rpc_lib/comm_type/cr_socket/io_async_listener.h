#ifndef IO_ASYNC_LISTENER_H
#define IO_ASYNC_LISTENER_H

#include <stdlib.h>
namespace cr_common {
    class io_async_listener
    {
    public:
        virtual ~io_async_listener(){}

    public:
        virtual void read_done(char* buf, ssize_t size) = 0;
        virtual void write_done(ssize_t size) = 0;
    };
}

#endif // IO_ASYNC_LISTENER_H
