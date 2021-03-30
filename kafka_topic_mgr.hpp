//
//  kafka_topic_mgr.hpp
//  test_kafka
//
//  Created by 沈佳锋 on 2021/3/2.
//  Copyright © 2021 沈佳锋. All rights reserved.
//

#ifndef kafka_topic_mgr_hpp
#define kafka_topic_mgr_hpp

#include <stdio.h>
#include <string>

class KafkaTopicMgr
{
public:
    virtual std::string client_topic() = 0;

    virtual std::string client_group_id() = 0;

    virtual std::string service_topic() = 0;

    virtual std::string service_group_id() = 0;
    
    virtual ~KafkaTopicMgr() = default;
};

#endif /* kafka_topic_mgr_hpp */
