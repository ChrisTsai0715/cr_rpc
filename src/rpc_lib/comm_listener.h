#ifndef COMM_LISTENER_H
#define COMM_LISTENER_H

namespace cr_rpc {

    class comm_base_listener
    {
    public:
        virtual ~comm_base_listener() {}
        virtual void _data_receive(int fd, char* buf, size_t size) = 0;
        virtual void _data_send(int fd, size_t size) = 0;
    };

    class comm_server_listener : public comm_base_listener
    {
    public:
        virtual ~comm_server_listener() {}
        virtual void _client_connect(int client_fd) = 0;
        virtual void _client_disconnect(int client_fd) = 0;
    };

    class comm_client_listener : public comm_base_listener
    {
    public:
        virtual ~comm_client_listener() {}
        virtual void _connect(int socket_fd) = 0;
        virtual void _disconnect_server(int socket_fd) = 0;
    };
}


#endif // COMM_LISTENER_H
