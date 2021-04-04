//
//  rpc_server.hpp
//  test_kafka
//
//  Created by 沈佳锋 on 2021/3/3.
//  Copyright © 2021 沈佳锋. All rights reserved.
//

#ifndef rpc_server_hpp
#define rpc_server_hpp

#include <stdio.h>
#include <string>

//错误码定义
#define RPC_SERVICE_INIT_ERROR 0x10000              //初始化失败
#define RPC_SERVICE_REGISTER_ERROR 0x10001          //注册失败，重复注册


/**
 *@brief rpc回调借口
 */
class RpcInterface
{
public:
    /**
     *@brief 接口名称
     *@return 返回接口名称
     */
    virtual std::string interface_name() = 0;
    
    /**
     * @brief 服务端rpc回调函数，服务端异常支持抛rpc_exception和返回值两种类型
     * @parameter request
     * @parameter response
     * @return 成功返回0，失败返回-1
     */
    virtual int rpc_service_cb(const std::string &request, std::string &response) = 0;
    
    virtual ~RpcInterface() = default;
};

class RpcService
{
public:
    /**
     * @brief 注册回调接口
     * @parameter rpc_cb回调函数
     */
    virtual void register_service_cb(RpcInterface *rpc_cb) = 0;
    
    /**
     * @brief: 服务器开启监听
     * @throw: 失败则抛出异常。每个service只能start一次，重复start将会抛出异常
     */
    virtual void start() = 0;

    virtual ~RpcService() = default;
};

class KafkaBrokerMgr;
class KafkaTopicMgr;
RpcService * create_rpc_service(KafkaBrokerMgr*, KafkaTopicMgr*);

#endif /* rpc_server_hpp */
