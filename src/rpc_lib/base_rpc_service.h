#ifndef IMYRPCSERVICE_H
#define IMYRPCSERVICE_H

#include <map>
#include <string>
#include <functional>

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
    typedef std::function<int(const std::map<std::string, std::string>&, std::map<std::string, std::string>&)> rpc_service_func_def;
    class base_rpc_service
    {
    public:
        explicit base_rpc_service(const std::string& service_name)
            :   _service_name(service_name)
        {
        }
        virtual ~base_rpc_service(){}
        const std::string& get_service_name() const {return _service_name;}
        virtual bool invoke(std::map<std::string, std::string>& args_map, std::map<std::string, std::string>& ack_map)
        {
            if (_rpc_funcs.find(args_map["cmd"]) == _rpc_funcs.end())
                return false;
            rpc_service_func_def func = _rpc_funcs[args_map["cmd"]];
            try
            {
                return func(args_map, ack_map) == 0;
            }
            catch(...)
            {
                return false;
            }
        }

    protected:
        virtual bool _reg_funcs(const std::string& name, rpc_service_func_def func)
        {
            if (_rpc_funcs.find(name) != _rpc_funcs.end()) return false;

            _rpc_funcs[name] = func;
            return true;
        }

    private:
        std::string _service_name;
        std::map<std::string, rpc_service_func_def>  _rpc_funcs;
    };
}
#endif // IMYRPCSERVICE_H
