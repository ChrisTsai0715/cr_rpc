#ifndef FIFO_COMM_H
#define FIFO_COMM_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdexcept>
#include "base_comm.h"

namespace cr_rpc
{
    class fifo_comm
    {
    public:
        fifo_comm()
            :	_write_fd(-1),
                _read_fd(-1)
        {

        }

        virtual ~fifo_comm()
        {

        }

    protected:
        virtual int _create_path(const std::string& path);

    protected:
        int _write_fd;
        int _read_fd;
    };

    class fifo_comm_server : protected fifo_comm,
                             public base_comm_server
    {
    public:
        fifo_comm_server(comm_server_listener* listener)
            :	base_comm_server(listener),
                _accept_fd(-1),
                _client_pid(0),
                _accept_flag(false)
        {

        }

        virtual ~fifo_comm_server(){}

        virtual bool accept();
        virtual bool read(int client_fd);
        virtual bool write(int client_fd, const char* buf, size_t size);

        virtual bool start_listen(const std::string& path);
        virtual bool accept_done(const char *buf, ssize_t size);
        virtual bool stop_listen();
        virtual bool disconnect_client();

        virtual bool _read_done(int id, char*buf, ssize_t size);
    private:
        int _accept_fd;
        int _client_pid;
        bool _accept_flag;
    };

    class fifo_comm_client : protected fifo_comm,
                             public base_comm_client
    {
    public:
        fifo_comm_client(comm_client_listener* listener)
            :	base_comm_client(listener),
                _connect_fd(-1)
        {

        }
        virtual ~fifo_comm_client(){}

        virtual bool start_connect(const std::string& path, unsigned int timeout_ms);
        virtual bool disconnect_server();
        virtual bool read();
        virtual bool write(const char *buf, size_t size);

        virtual bool _read_done(int id, char* buf, ssize_t size);

    private:
        bool _connect_flag;
        int  _connect_fd;
    };
}
#endif // FIFO_COMM_H
