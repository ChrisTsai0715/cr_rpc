#ifndef IO_FD_H
#define IO_FD_H

#include "io_async_listener.h"
#include "select_tracker.h"
#include "select_task.h"
#include "IReference.h"
#include "cslock.h"

namespace cr_common {

    class io_fd;

    class io_select_event
    {
    public:
        io_select_event(cr_common::io_fd* listener)
            :   _listener(listener)
        {
        }

    protected:
        template<typename T>
        T* _get_listener()
        {
            if (_listener == 0)
                return NULL;

            return dynamic_cast<T*>(_listener);
        }

    protected:
        cr_common::io_fd* _listener;
    };

    class io_fd : public c_ref
    {
    public:
        explicit io_fd(ref_obj<cr_common::select_tracker> tracker);
        explicit io_fd(int fd, ref_obj<cr_common::select_tracker> tracker);
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
        ref_obj<cr_common::select_tracker> _tracker;
        int _fd;

    private:
        CMutexLock _mutex;

    private:
        class read_task : public select_task,
                          public io_select_event
        {
        public:
            static read_task* new_instance(int read_fd, cr_common::io_fd* listener)
            {
                return new read_task(read_fd, listener);
            }

        private:
            read_task(int read_fd, cr_common::io_fd* listener)
                :   select_task(select_task::TASK_SELECT_READ, read_fd),
                    io_select_event(listener)
            {

            }

            virtual ~read_task(){}
            virtual bool done();
        };

        class write_task : public select_task,
                           public io_select_event
        {
        public:
            static write_task* new_instance(int write_fd, cr_common::io_fd* listener, const char* buf, size_t size)
            {
                return new write_task(write_fd, listener, buf, size);
            }

        private:
            write_task(int write_fd, cr_common::io_fd* listener, const char* buf, size_t size)
                :   select_task(select_task::TASK_SELECT_WRITE, write_fd),
            io_select_event(listener),
            _send_size(size)
            {
                _send_buf = new char[size];
                ::memcpy(_send_buf, buf, size);
            }
            virtual ~write_task(){}
            virtual bool done();

        private:
            char*  _send_buf;
            size_t _send_size;
        };
    };
}

#endif // IO_FD_H
