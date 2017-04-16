#ifndef INET_SOCKET_H
#define INET_SOCKET_H

#include "net_socket.h"
#include "base_comm.h"
#include "comm_listener.h"
#include "select_task.h"

namespace cr_common {

    /*****************
     * inet_socket
     *****************/
    class inet_socket : public net_socket
    {
    public:
        inet_socket(int socket_fd)
            :	net_socket(socket_fd)
        {

        }

        inet_socket(stream_type type)
            :	net_socket(type)
        {

        }

        inet_socket(stream_type type,
                    CRefObj<select_tracker> tracker
                    )
            :	net_socket(tracker, type)
        {

        }

        virtual ~inet_socket(){}

    public:
        virtual int init();

    protected:
        virtual int _bind(const std::string& addr);
        //get domain and port from addr
        virtual bool _acquire_domain_port(const std::string& addr, std::string& domain, uint16_t& port);
        ssize_t recv_data(char* buf, size_t size);
        ssize_t send_data(const char* buf, size_t size);

    public:
        //interface for base_comm
        virtual void _on_data_receive(char* buf, ssize_t size);
        virtual void _on_data_send(ssize_t size);
    };

    /******************
     * inet_server
     ******************/
    class inet_server : public inet_socket,
                        public base_comm_server
    {
    public:
        explicit inet_server(stream_type type)
            :	inet_socket(type)
        {

        }

        inet_server(stream_type type,
                   CRefObj<cr_common::select_tracker> tracker,
                   comm_server_listener* listener
                   )
            :	inet_socket(type, tracker),
                base_comm_server(listener)
        {

        }

        virtual ~inet_server(){}

    public:
        virtual int start_listen(const std::string& path);
        virtual int stop_listen();

    private:
        virtual int _accept();

    public:
        //interface for base_comm_server
        virtual void _on_connect(io_fd* client);
        virtual ssize_t recv_data(char* buf, size_t size);
        virtual ssize_t send_data(const char* buf, size_t size);

    public:
        //interface for io_fd
        virtual void _on_read_done(char* buf, ssize_t size);
        virtual void _on_write_done(ssize_t size);
    private:
         /***************
         * socket_accept_task
         ***************/
        class socket_accept_task : public select_task,
                                   public comm_select_event
        {
        public:
            static socket_accept_task* new_instance(int listen_fd, base_comm* listener)
            {
                return new socket_accept_task(listen_fd, listener);
            }

        private:
            socket_accept_task(int listen_fd, base_comm* listener)
                :   select_task(select_task::TASK_SELECT_ACCEPT, listen_fd),
                    comm_select_event(listener)
            {
            }

            virtual ~socket_accept_task(){}
            virtual bool done();
        };
    };

    /*****************
     * inet_client
     *****************/
    class inet_client : public inet_socket,
                        public base_comm_client
    {
    public:
        explicit inet_client(stream_type type)
            :	inet_socket(type)
        {

        }

        inet_client(stream_type type,
                   CRefObj<cr_common::select_tracker> tracker,
                   comm_client_listener* listener
                   )
            :	inet_socket(type, tracker),
                base_comm_client(listener)
        {

        }

        virtual ~inet_client(){}

    public:
        //interface for base_comm_client
        virtual int start_connect(const std::string& path, unsigned int timeout_ms);
        virtual int disconnect_server() = 0;
        virtual ssize_t recv_data(char* buf, size_t size);
        virtual ssize_t send_data(const char* buf, size_t size);

    private:
        /*********************
         * socket_connect_task
         *********************/
        class socket_connect_task : public select_task,
                                    public comm_select_event
        {
        public:
            static socket_connect_task* new_instance(int connect_fd, base_comm* listener, const std::string& dest_addr, uint16_t port)
            {
                return new socket_connect_task(connect_fd, listener, dest_addr, port);
            }

        private:
            socket_connect_task(int connect_fd, base_comm* listener, const std::string& dest_addr, uint16_t port)
                :   select_task(select_task::TASK_SELECT_ACCEPT, connect_fd),
                    comm_select_event(listener),
                    _dest_addr(dest_addr),
                    _port(port)
            {

            }

            virtual ~socket_connect_task(){}
            virtual bool done();
        private:
            std::string _dest_addr;
            uint16_t _port;
        };
    };

}
#endif // INET_SOCKET_H
