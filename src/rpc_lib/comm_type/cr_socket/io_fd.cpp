#include "io_fd.h"
#include "select_task.h"

using namespace cr_common;

io_fd::io_fd(io_async_listener *listener, CRefObj<cr_rpc::select_tracker> tracker)
    :	_tracker(tracker),
        _listener(listener),
        _fd(0)
{

}

io_fd::io_fd()
    :	_tracker(0),
        _fd(0)
{

}

io_fd::~io_fd()
{
    if (_fd > 0)
        ::close(_fd);
    _fd = 0;
}

ssize_t io_fd::read(char *buf, size_t size)
{
    return this->_read(buf, size);
}

ssize_t io_fd::async_read()
{
    return this->_read(NULL, 2048, false);
}

ssize_t io_fd::write(const char *buf, size_t size)
{
    return _write(buf, size);
}

ssize_t io_fd::async_write(const char *buf, size_t size)
{
    return _write(buf, size, false);
}

ssize_t io_fd::_write(const char *buf, size_t size, bool block)
{
    CAutoLock<CMutexLock> cslock(_mutex);
    if (_fd == 0) return -1;

    ssize_t write_size = 0;
    do
    {
        write_size = ::write(_fd, buf, size);
        if(write_size == -1)
        {
            if (!block && (errno != EAGAIN && errno != EWOULDBLOCK))
            {
                return _tracker->add_task(
                            cr_rpc::write_task::new_instance(_fd, _listener, buf, size)
                            ) ? 0 : -1;
            }

            return -1;
        }
        if (!block) _listener->write_done(write_size);
        size -= write_size;
        buf  += write_size;
    }while(write_size > 0 && size > 0);

    return !block ? 0 : write_size;
}

ssize_t io_fd::_read(char *buf, size_t size, bool block)
{
    CAutoLock<CMutexLock> cslock(_mutex);
    if (_fd == 0) return -1;
    ssize_t read_size = 0;
    do
    {
        char read_buf[2048] = {0};
        !block ?
            read_size = ::read(_fd, read_buf, sizeof read_buf)
                :
            read_size = ::read(_fd, buf, size);

        if (read_size == -1)
        {
            if (!block && (errno == EAGAIN || errno == EWOULDBLOCK))
            {
                return _tracker->add_task(
                            cr_rpc::read_task::new_instance(_fd, _listener)
                            ) ? 0 : -1;
            }

            return -1;
        }

        if (!block) _listener->read_done(read_buf, read_size);

    }while((size_t)read_size == size);

    return !block ? 0 : read_size;
}
