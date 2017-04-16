#ifndef BASE_RPC_INTERFACE_H
#define BASE_RPC_INTERFACE_H

#include <stdio.h>
#include <string>
#include <string.h>
#include <map>
#include <errno.h>
#include <stdexcept>
#include <stddef.h>
#include <unistd.h>
#include <assert.h>
#include <list>
#include <functional>
#include "base_rpc_service.h"
#include "cr_socket/inet_socket.h"
#include "common/base_thread.h"
#include "common/ccondition.h"
#include "common/IReference.h"
#include "json/value.h"

namespace cr_common
{
    typedef enum
    {
        RPC_COMM_TYPE_FIFO,
        RPC_COMM_TYPE_INET,
        RPC_COMM_TYPE_UNIX,
    }rpc_comm_type_def;

    typedef std::map<std::string, std::string> rpc_req_args_type;

    class base_rpc
    {
    public:
        base_rpc();
        virtual ~base_rpc();

        void set_unix_sock_path(const std::string& path) {_socket_path = path;}
        bool reg_services(const std::string& service_name, base_rpc_service* service);

    protected:
        bool _format_req_msg(const std::string &cmd, rpc_req_args_type &req_map, std::string &value);
        bool _req_msg_parse(const char* req_msg, size_t size, std::vector<rpc_req_args_type>& req_map_vec);
        bool _receive_data_handle(char *buf, size_t size);

    public:
        virtual bool send_req(const std::string& cmd, rpc_req_args_type& req_map) = 0;

    private:
        int _divide_req_msg(const void *req_msg, size_t size, std::list<Json::Value> &json_value_list, void **unread_ptr);

    private:
        char _divide_unread_buf[2048];
        size_t _divide_unread_size;

    protected:
        class rpc_data_handle_thread : private base_thread
        {
        public:
            rpc_data_handle_thread(base_rpc* outer)
                :   _outer(outer)
            {}
            virtual ~rpc_data_handle_thread(){}

            virtual bool run()
            {
                _req_map_lists.clear();
                return base_thread::run();
            }

            virtual bool stop()
            {
                cr_common::auto_cond cscond(_req_map_cond);
                _req_map_cond.signal();
                return base_thread::stop();
            }

            bool _push_req_map(const rpc_req_args_type &req_map);
            template<typename T>
            bool _push_req_map(T& req_maps);

        private:
            virtual bool thread_loop();

        private:
            cr_common::condition _req_map_cond;
            std::list<rpc_req_args_type>_req_map_lists;
            base_rpc* _outer;
        }_rpc_handler;

    protected:
        std::string _socket_path;
        std::map<std::string, base_rpc_service*> _services;

    friend class rpc_data_handle_thread;
    };
}
#endif // MYRPCINTERFACE_H
