//
//  rpc_message.hpp
//  test_kafka
//
//  Created by 沈佳锋 on 2021/3/3.
//  Copyright © 2021 沈佳锋. All rights reserved.
//

#ifndef rpc_message_hpp
#define rpc_message_hpp

#include <stdio.h>

#include <string>
#include <sstream>

class RpcMessage
{
private:
    std::string interface_name_;
    std::string response_topic_;
    std::string id_;
    std::string data_;
    
public:
    RpcMessage() {}
    
    void set_id(std::string id)
    {
        id_ = id;
    }
    
    std::string id()
    {
        return id_;
    }
    
    void set_response_topic(std::string name)
    {
        response_topic_ = name;
    }
    
    std::string response_topic()
    {
        return response_topic_;
    }
    
    void set_interface_name(std::string name)
    {
        interface_name_ = name;
    }
    
    void set_data(std::string data)
    {
        data_ = data;
    }
    
    std::string interface_name()
    {
        return interface_name_;
    }
    
    std::string data()
    {
        return data_;
    }
    
    void from_string(std::string message)
    {
        unsigned int pos = message.find(":");
        interface_name_ = message.substr(0, pos);
        std::string tmp_data = message.substr(pos + 1);
        pos = tmp_data.find(":");
        response_topic_ = tmp_data.substr(0, pos);
        tmp_data  = tmp_data.substr(pos + 1);
        pos = tmp_data.find(":");
        id_ = tmp_data.substr(0, pos);
        data_ = tmp_data.substr(pos+1);
    }
    
    std::string to_string()
    {
        std::stringstream ss;
        ss << interface_name_ << ":"<< response_topic_ << ":" << id_ <<":" << data_;
        return ss.str();
    }
};

#endif /* rpc_message_hpp */
