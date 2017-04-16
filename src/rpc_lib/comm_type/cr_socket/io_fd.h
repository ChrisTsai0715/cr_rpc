#ifndef IO_FD_H
#define IO_FD_H

#include "io_async_listener.h"
#include "select_tracker.h"
#include "IReference.h"
#include "cslock.h"

namespace cr_common {

    class io_fd : public CReference
    {
    public:
        explicit io_fd(CRefObj<cr_common::select_tracker> tracker);
        explicit io_fd(int fd);
        io_fd();
        virtual ~io_fd();

        virtual int init() = 0;
        virtual int close() = 0;

        virtual ssize_t read(char* buf, size_t size);
        virtual ssize_t write(const char* buf, size_t size);
        virtual ssize_t async_read();
        virtual ssize_t async_write(const char* buf, size_t size);
        operator int() const{return _fd;}

    public:
        virtual void on_read_done(char* buf, ssize_t size) = 0;
        virtual void on_write_done(ssize_t size) = 0;

    private:
        ssize_t _write(const char*buf, size_t size, bool block = true);
        ssize_t _read(char* buf, size_t size, bool block = true);

    protected:
        CRefObj<cr_common::select_tracker> _tracker;
        int _fd;

    private:
        CMutexLock _mutex;
   };
}

#endif // IO_FD_H
