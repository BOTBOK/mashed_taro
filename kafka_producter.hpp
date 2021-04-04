//
//  kafka_product.hpp
//  test_kafka
//
//  Created by 沈佳锋 on 2021/3/4.
//  Copyright © 2021 沈佳锋. All rights reserved.
//

#ifndef kafka_product_hpp
#define kafka_product_hpp

#include <stdio.h>
#include <string>


class KafkaProducter
{
public:
    /**
     * @brief 往指定topic发送数据
     * @parameter: topic
     * @parameter: data
     * @return: 成功投递返回0，失败返回-1
     */
    virtual int product_message(const std::string topic, const std::string data) = 0;
    
    /**
     * @brief 初始化kafka消费类，一个对象只能初始化一次，重复初始化会返回错误
     * @parameter brokers 集群结点列表，string类型，格式为ip:port,ip:port,ip:port
     * @return 0: 初始化成功， -1:初始化失败
     */
    virtual int init(std::string brokers) = 0;
    
    /**
     * @brief 释放资源，每个对象只能释放一次，重复释放将会返回失败
     * @return 0:释放成功，-1:释放失败
     */
    virtual int release() = 0;
    
    virtual ~KafkaProducter() = default;
};

KafkaProducter* create_kafka_producter();

#endif /* kafka_product_hpp */
