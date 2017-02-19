#ifndef BASE_COMM_H
#define BASE_COMM_H

#include "common/IReference.h"
#include "select_task.h"

namespace cr_rpc
{
    class comm_server_listener
    {
    public:
        virtual void _client_connect(int client_fd) = 0;
        virtual void _client_disconnect(int client_fd) = 0;
        virtual void _client_data_receive(int socket_fd, char* buf, size_t size) = 0;
        virtual void _client_data_send(int socket_fd, size_t size) = 0;
    };

    class comm_client_listener
    {
    public:
        virtual void _disconnect_server(int socket_fd) = 0;
        virtual void _data_receive(int socket_fd, char* buf, size_t size) = 0;
        virtual void _data_send(int socket_fd, size_t size) = 0;
    };

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
                        typedef bool(base_comm::*func)(int, char *, ssize_t);
                        return _select_tracker.add_task(
                                    read_task<base_comm*, func>::new_instance(id, this, &base_comm::_read_done)
                                    );
                        return true;
                    }
                    return false;
                }
                _read_done(id, read_buf, read_size);
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
                        typedef bool(base_comm::*func)(int, ssize_t);
                        return _select_tracker.add_task(
                               write_task<base_comm*, func>::new_instance(id, this, &base_comm::_write_done, buf, size)
                               );
                        return true;
                    }

                    return false;
                }
                _write_done(id, write_size);
                size -= write_size;
                buf  += write_size;
            }while(write_size > 0 && size > 0);

            return true;
        }

    public:
        virtual bool _read_done(int, char*, ssize_t) = 0;
        virtual bool _write_done(int, ssize_t) = 0;

    protected:
        select_tracker _select_tracker;
        CMutexLockEx _read_mutex;
        CMutexLockEx _write_mutex;
    };

    class base_comm_client : public base_comm
    {
    public:
        base_comm_client(comm_client_listener* listener)
            :	_listener(listener)
        {

        }
        virtual ~base_comm_client()
        {

        }

        virtual bool _read_done(int id, char* buf, ssize_t size)
        {
            if (_listener)
            {
                size == 0 ? _listener->_disconnect_server(id)
                                :
                            _listener->_data_receive(id, buf, size);
            }

            return true;
        }

        virtual bool _write_done(int id, ssize_t size)
        {
            if (_listener)
            {
                size == -1 ? _listener->_disconnect_server(id)
                                :
                             _listener->_data_send(id, size);
            }

            return true;
        }

        virtual bool start_connect(const std::string& path, unsigned int timeout_ms) = 0;
        virtual bool disconnect_server() = 0;

        virtual bool read() = 0;
        virtual bool write(const char *buf, size_t size) = 0;

    protected:
        comm_client_listener* _listener;
    };

    class base_comm_server : public base_comm
    {
    public:
        base_comm_server(comm_server_listener* listener)
            :	_listener(listener)
        {

        }
        virtual ~base_comm_server()
        {

        }

        virtual bool _read_done(int id, char* buf, ssize_t size)
        {
            if (_listener)
            {
                size == 0 ? _listener->_client_disconnect(id)
                                :
                            _listener->_client_data_receive(id, buf, size);
            }

            return true;
        }

        virtual bool _write_done(int id, ssize_t size)
        {
            if (_listener)
            {
                size == -1 ? _listener->_client_disconnect(id)
                                :
                             _listener->_client_data_send(id, size);
            }

            return true;
        }

        virtual bool start_listen(const std::string&) = 0;
        virtual bool stop_listen() = 0;

        virtual bool accept() = 0;
        virtual bool read() = 0;
        virtual bool write(const char*, size_t) = 0;
        virtual bool disconnect_client() = 0;

    protected:
        comm_server_listener* _listener;
    };
}

#endif // BASE_COMM_H
