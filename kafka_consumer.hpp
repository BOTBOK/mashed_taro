//
//  kafka_consumer.hpp
//  test_kafka
//
//  Created by 沈佳锋 on 2021/3/3.
//  Copyright © 2021 沈佳锋. All rights reserved.
//

#ifndef kafka_consumer_hpp
#define kafka_consumer_hpp


#include <string>
#include <functional>
typedef std::function<void (std::string&)> ConsumerCbFunc;
/**
 * @brief kafka消费类，每个对象维护一个kafka连接。用于消费指定topic的数据
 */
class KafkaConsumer
{
public:
    /**
     * @brief 订阅topic，每个topic对应一个回调函数。当执行start_consumer后不允许注册回调
     * @parameter topic 需要订阅的主题
     * @parameter func 回调函数
     * @return 0:表示注册成功，-1:表示注册失败。
     */
    virtual int subscribe_topic(std::string topic, ConsumerCbFunc func) = 0;
    
    /**
     * @brief 初始化kafka消费类，一个对象只能初始化一次，重复初始化会返回错误
     * @parameter brokers 集群结点列表，string类型，格式为ip:port,ip:port,ip:port
     * @return 0: 初始化成功， -1:初始化失败
     */
    virtual int init(std::string brokers, std::string group_id) = 0;
    
    /**
     * @brief 启动消费，每个对象只能启动一次消费
     * @return 0: 启动成功， -1:启动失败
     */
    virtual int start_consumer() = 0;
    
    /**
     * @brief 释放资源，每个对象只能释放一次，重复释放将会返回失败
     * @return 0:释放成功，-1:释放失败
     */
    virtual int release() = 0;
    
    virtual ~KafkaConsumer() = default;

};

KafkaConsumer *create_kafka_consumer();


#endif /* kafka_consumer_hpp */
