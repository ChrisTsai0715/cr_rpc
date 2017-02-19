#ifndef IMYRPCSERVICE_H
#define IMYRPCSERVICE_H

#include <map>
#include <string>

namespace cr_rpc
{
    /**************
     *
     * base_rpc_service
     *
     * usage : Should create a class to inherit base_rpc_service.
     * 		   Should create virtual function invoke.
     *
     * ************/
    class base_rpc_service
    {
    public:
        explicit base_rpc_service(const std::string& service_name)
            :   _service_name(service_name)
        {
        }
        virtual ~base_rpc_service(){}
        const std::string& get_service_name() const {return _service_name;}
        virtual bool invoke(std::map<std::string, std::string>& args_map, std::map<std::string, std::string>& ack_map) = 0;

    private:
        std::string _service_name;
    };
}
#endif // IMYRPCSERVICE_H
