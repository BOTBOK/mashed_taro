//
//  kafka_broker_mgr.cpp
//  test_kafka
//
//  Created by 沈佳锋 on 2021/3/2.
//  Copyright © 2021 沈佳锋. All rights reserved.
//
#if 0

#include "kafka_broker_mgr.hpp"


class KafkaBrokerMgrImpl: public KafkaBrokerMgr
{
public:
    std::string get_zk_list() override
    {
        return "10.50.38.233:2181,10.50.38.234:2181,10.50.38.235:2181";
    }
    
    std::string get_broker_list() override
    {
        return "10.50.38.233:9092,10.50.38.234:9092,10.50.38.235:9092";
    }
    
};

static KafkaBrokerMgrImpl kafka_broker_mgr_impl = KafkaBrokerMgrImpl();

KafkaBrokerMgr* KafkaBrokerMgr::instance()
{
    return &kafka_broker_mgr_impl;
}
#endif