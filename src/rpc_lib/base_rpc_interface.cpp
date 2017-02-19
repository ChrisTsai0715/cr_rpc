#include "base_rpc_interface.h"
#include "json/json.h"
#include "json/reader.h"
#include "json/writer.h"

using namespace cr_rpc;
/***************************************************
 *
 *          my_rpc_interface
 *
 * *************************************************/
base_rpc_interface::base_rpc_interface()
    :   _divide_unread_size(0),
        _rpc_handler(this),
        _socket_path("/tmp/")
{

}

base_rpc_interface::~base_rpc_interface()
{
    _rpc_handler.stop();
}

bool base_rpc_interface::_receive_data_handle(char *buf, size_t size)
{
    std::vector<rpc_req_args_type> rpc_req_args_vec;
    if (!_req_msg_parse(buf, size, rpc_req_args_vec))
        return false;

    return _rpc_handler._push_req_map<std::vector<rpc_req_args_type>, std::vector<rpc_req_args_type>::iterator>(rpc_req_args_vec);
}

bool base_rpc_interface::_reg_services(const std::string &service_name, base_rpc_service *service)
{
    assert(((u_int64_t)service) * service_name.size());

    _services[service_name] = service;

    return true;
}

bool base_rpc_interface::_format_req_msg(const std::string& cmd, rpc_req_args_type &req_map, std::string &value)
{
    assert(cmd.size());

    Json::Value req_json;
    req_json["rpc_service"] = cmd;
    Json::Value arg_json;
    rpc_req_args_type::iterator it = req_map.begin();
    for (; it != req_map.end(); it ++)
    {
        arg_json[it->first] = it->second;
    }
    req_json["args"] = arg_json;

    Json::FastWriter writer;
    value = writer.write(req_json);

    return true;
}

bool base_rpc_interface::_req_msg_parse(const char *req_msg, size_t size, std::vector<rpc_req_args_type> &req_map_vec)
{
    std::list<Json::Value> json_value_list;
    void* unread_ptr = 0;
    if (_divide_unread_size > 0)
    {
        memcpy(_divide_unread_buf + _divide_unread_size, req_msg, size);
        if (_divide_req_msg(_divide_unread_buf, size + _divide_unread_size, json_value_list, &unread_ptr) < 0)
            return false;
    }
    else
    {
        if (_divide_req_msg(req_msg, size, json_value_list, &unread_ptr) < 0)
            return false;
    }

    if (unread_ptr == 0)
    {
        _divide_unread_size = 0;
    }
    else
    {
        _divide_unread_size = size + _divide_unread_size - ((u_int64_t)unread_ptr - (u_int64_t)_divide_unread_buf);
        memcpy(_divide_unread_buf, unread_ptr, _divide_unread_size);
    }

    std::list<Json::Value>::iterator json_it = json_value_list.begin();
    for (; json_it != json_value_list.end(); json_it ++)
    {
        rpc_req_args_type req_args_map;
        req_args_map["rpc_service"] = (*json_it)["rpc_service"].asString();
        Json::Value req_args_json   = (*json_it)["args"];

        Json::Value::Members membrs = req_args_json.getMemberNames();
        Json::Value::Members::iterator mbr_it = membrs.begin();
        for (; mbr_it != membrs.end(); mbr_it ++)
        {
            req_args_map[*mbr_it] = req_args_json[*mbr_it].asString();
        }

        req_map_vec.push_back(req_args_map);
    }

    return true;
}

int base_rpc_interface::_divide_req_msg(const void *req_msg, size_t size, std::list<Json::Value>& json_value_list, void **unread_ptr)
{
    size_t index = 0;
    for (; index < size; index ++)
    {
        if (*(char *)(((char *)req_msg) + index) == '\n')
            break;
    }
    if (index == size)
    {
        *unread_ptr = (void *)req_msg;
        return -1;
    }
    if (index == 0)
    {
        return -2;
    }
    Json::Reader reader;
    Json::Value json_value;
    if (reader.parse(std::string((char *)req_msg, index), json_value))
        json_value_list.push_back(json_value);
    else
        return -100;

    if (0 == size - index - 1)
        return 0;

    return this->_divide_req_msg(((char *)req_msg) + index + 1, size - index - 1, json_value_list, unread_ptr);
}

bool base_rpc_interface::rpc_data_handle_thread::_push_req_map(const rpc_req_args_type &req_map)
{
    cr_common::auto_cond cscond(_req_map_cond);
    _req_map_lists.push_back(req_map);
    return _req_map_cond.signal() == 0;
}

template<typename T, typename K>
bool base_rpc_interface::rpc_data_handle_thread::_push_req_map(T &req_maps)
{
    if (req_maps.size() == 0) return true;
    cr_common::auto_cond cscond(_req_map_cond);
    K it = req_maps.begin();
    for(; it != req_maps.end(); it ++)
    {
        _req_map_lists.push_back(*it);
    }
    return _req_map_cond.signal() == 0;
}

bool base_rpc_interface::rpc_data_handle_thread::thread_loop()
{
    cr_common::auto_cond cscond(_req_map_cond);
    if (_req_map_lists.size() == 0)
    {
        if (_req_map_cond.time_wait(1000) == 0)
        {
            if (_req_map_lists.size() == 0)
                return true;
        }
        else return true;
    }

    std::list<rpc_req_args_type>::iterator it = _req_map_lists.begin();
    for (; it != _req_map_lists.end(); it ++)
    {
        if (_outer->_services.find((*it)["rpc_service"]) == _outer->_services.end())
            continue;
        base_rpc_service* p = _outer->_services[(*it)["rpc_service"]] ;
        rpc_req_args_type ack_map;
        p->invoke(*it, ack_map);
        if (ack_map.size() > 0)
        {
            _outer->send_req((*it)["rpc_service"], ack_map);
        }
    }

    _req_map_lists.clear();

    return true;
}
