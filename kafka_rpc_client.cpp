//
//  rpc_client.cpp
//  test_kafka
//
//  Created by 沈佳锋 on 2021/3/5.
//  Copyright © 2021 沈佳锋. All rights reserved.
//

#include "kafka_rpc_client.hpp"
#include "kafka_rpc_message.hpp"
#include "kafka_broker_mgr.hpp"
#include "kafka_topic_mgr.hpp"
#include "kafka_producter.hpp"
#include "kafka_consumer.hpp"
#include "kafka_rpc_exception.hpp"
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include "kafka_broker_mgr.hpp"

class RpcClientImpl:public RpcClient
{
private:
    uint64_t index_;
    
    KafkaProducter *kafka_producter_;
    KafkaConsumer *kafka_cunsumer_;

    typedef std::function<void(RpcMessage &msg)> rpc_func;
    
    std::mutex func_map_mutex_;
    std::unordered_map<std::string, rpc_func> func_map_;

    std::string brokers_;
    std::string client_topic_;
    std::string service_topic_;

private:
    std::string get_id()
    {
        char tmp_buff[100];
        snprintf(tmp_buff, 100, "%llu", index_++);
        return tmp_buff;
    }
    
    
public:
 
    void deal_response(RpcMessage &msg, RpcMessage *res_msg, std::condition_variable *cond)//,,)
    {
        //最简单的数据处理
        *res_msg = msg;
   	    cond->notify_all();
    }
    
    void call(const std::string &interface_name, const std::string &request, std::string &response)
    {
        RpcMessage msg;
        msg.set_id(get_id());
        msg.set_interface_name(interface_name);

        msg.set_response_topic(client_topic_); 
        
        std::condition_variable cv;
        RpcMessage response_msg;
        {
            std::lock_guard<std::mutex> lock(func_map_mutex_);
	    func_map_.emplace(msg.id(),std::bind(&RpcClientImpl::deal_response, this, std::placeholders::_1, &response_msg, &cv));
        }
        
        if(nullptr != kafka_producter_)
        {
            kafka_producter_->product_message(service_topic_, msg.to_string() );
        }
        
        std::mutex tmp_mutex_;
        std::unique_lock<std::mutex> tmp_lock(tmp_mutex_);
        cv.wait(tmp_lock);
        
        response = response_msg.data();
    }
   
    void deal(std::string &data)
    {
        RpcMessage msg;
        msg.from_string(data);
        rpc_func _func = nullptr;
        {
            std::lock_guard<std::mutex> lock(func_map_mutex_);
            std::unordered_map<std::string, rpc_func>::iterator _find = func_map_.find(msg.id());
            if(_find != func_map_.end())
                _func = _find->second;
        }
        if(nullptr != _func)
            _func(msg);
    }
    
    RpcClientImpl(KafkaBrokerMgr* broker_mgr, KafkaTopicMgr* topic_mgr): index_(0), 
        brokers_(broker_mgr->broker_list()), 
        client_topic_(topic_mgr->client_topic()),
        service_topic_(topic_mgr->service_topic())
    {
        kafka_producter_ = create_kafka_producter();
        kafka_cunsumer_ = create_kafka_consumer();
        RPC_THROW_IF(nullptr == kafka_producter_, -1, "create_kafka_producter error!");
        RPC_THROW_IF(nullptr == kafka_cunsumer_, -1, "create_kafka_consumer error!");
        
        //消费组生成随机数吧 todo
        kafka_cunsumer_->init(broker_mgr->broker_list(), topic_mgr->client_group_id());
         
        //初始化producter
        kafka_producter_->init(broker_mgr->broker_list());
        
        //订阅topic
        kafka_cunsumer_->subscribe_topic(topic_mgr->client_topic(), std::bind(&RpcClientImpl::deal, this, std::placeholders::_1));
        
        //开启监听
        RPC_THROW_IF(0!= kafka_cunsumer_->start_consumer(), -1, "start_consumer error!");
    }
    
    ~RpcClientImpl()
    {
        if(kafka_producter_ != nullptr)
        {
            delete kafka_producter_;
        }
    }
};


RpcClient *create_rpc_client(KafkaBrokerMgr* broker_mgr, KafkaTopicMgr* topic_mgr)
{
    return new RpcClientImpl(broker_mgr, topic_mgr);
}
