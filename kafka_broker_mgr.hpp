//
//  kafka_broker_mgr.hpp
//  test_kafka
//
//  Created by 沈佳锋 on 2021/3/2.
//  Copyright © 2021 沈佳锋. All rights reserved.
//

#ifndef kafka_broker_mgr_hpp
#define kafka_broker_mgr_hpp

#include <stdio.h>
#include <string>

class KafkaBrokerMgr
{
public:
    virtual std::string zk_list() = 0;
    
    virtual std::string broker_list() = 0;
    
    virtual ~KafkaBrokerMgr() = default;
};

#endif /* kafka_broker_mgr_hpp */
