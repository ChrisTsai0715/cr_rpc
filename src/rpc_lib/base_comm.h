#ifndef BASE_COMM_H
#define BASE_COMM_H

#include "common/IReference.h"
#include "select_task.h"
#include "comm_listener.h"

namespace cr_rpc
{
    class select_tracker;
    class read_task;
    class write_task;

    class base_comm : public CReference
    {
    public:
        base_comm()
        {
            _select_tracker.run();
        }

        virtual ~base_comm()
        {
            _select_tracker.stop();
        }

    protected:
        virtual bool _read(int id)
        {
            CAutoLock<CMutexLock> cslock(_read_mutex);
            if (id == -1) return false;
            char read_buf[2048] = {0};
            ssize_t read_size = 0;
            do
            {
                read_size = ::read(id, read_buf, sizeof read_buf);
                if (read_size == -1)
                {
                    if (errno == EAGAIN || errno == EWOULDBLOCK)
                    {
                        return _select_tracker.add_task(
                                    read_task::new_instance(id, this)
                                    );
                    }
                    return false;
                }
                if (_listener) _listener->_data_receive(id, read_buf, read_size);
                //_read_done(id, read_buf, read_size);
            }while(read_size == sizeof read_buf);

            return true;
        }
        virtual bool _write(int id, const char* buf, size_t size)
        {
            CAutoLock<CMutexLock> cslock(_write_mutex);
            if (id == -1) return false;

            ssize_t write_size = 0;
            do
            {
                write_size = ::write(id, buf, size);
                if(write_size == -1)
                {
                    if (errno != EAGAIN && errno != EWOULDBLOCK)
                    {
                        return _select_tracker.add_task(
                               write_task::new_instance(id, this, buf, size)
                               );
                    }

                    return false;
                }
                if (_listener) _listener->_data_send(id, write_size);
                //_write_done(id, write_size);
                size -= write_size;
                buf  += write_size;
            }while(write_size > 0 && size > 0);

            return true;
        }

    public:
  //      virtual bool _read_done(int, char*, ssize_t) = 0;
  //      virtual bool _write_done(int, ssize_t) = 0;

    protected:
        select_tracker _select_tracker;
        CMutexLockEx _read_mutex;
        CMutexLockEx _write_mutex;
    };

    class base_comm_client : public base_comm
    {
    public:
        base_comm_client()
        {

        }

        virtual ~base_comm_client()
        {

        }

        virtual bool start_connect(const std::string& path, unsigned int timeout_ms) = 0;
        virtual bool disconnect_server() = 0;

        virtual bool read() = 0;
        virtual bool write(const char *buf, size_t size) = 0;

    public:
        //interface for select task
        virtual void _on_connect(int client_fd) = 0;
        virtual void _on_disconnect(int client_fd) = 0;
        virtual void _on_data_receive(int fd, char* buf, size_t size) = 0;
        virtual void _on_data_send(int fd, size_t size) = 0;
    };

    class base_comm_server : public base_comm
    {
    public:
        base_comm_server()
        {

        }

        virtual ~base_comm_server()
        {

        }

        virtual bool start_listen(const std::string&) = 0;
        virtual bool stop_listen() = 0;

        virtual bool accept() = 0;
        virtual bool read(int client_fd) = 0;
        virtual bool write(int client_fd, const char*, size_t) = 0;
        virtual bool disconnect_client(int client_fd) = 0;

    public:
        //interface for select task
        virtual void _on_connect(int socket_fd) = 0;
        virtual void _on_disconnect_server(int socket_fd) = 0;
        virtual void _on_data_receive(int fd, char* buf, size_t size) = 0;
        virtual void _on_data_send(int fd, size_t size) = 0;
    };
}

#endif // BASE_COMM_H
