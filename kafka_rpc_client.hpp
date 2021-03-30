//
//  rpc_client.hpp
//  test_kafka
//
//  Created by 沈佳锋 on 2021/3/5.
//  Copyright © 2021 沈佳锋. All rights reserved.
//

#ifndef rpc_client_hpp
#define rpc_client_hpp

#include <stdio.h>
#include <string>
#include <functional>

class RpcClient
{
public:
    /**
     *@brief rpc调用
     */
    virtual void call(const std::string &interface_name, const std::string &request, std::string &response) = 0;
    
    virtual ~RpcClient() = default;
};

class KafkaBrokerMgr;
class KafkaTopicMgr;
RpcClient *create_rpc_client(KafkaBrokerMgr*, KafkaTopicMgr*);

#endif /* rpc_client_hpp */
