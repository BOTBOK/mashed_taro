//
//  kafka_topic_mgr.cpp
//  test_kafka
//
//  Created by 沈佳锋 on 2021/3/2.
//  Copyright © 2021 沈佳锋. All rights reserved.
//
#if 0
#include "kafka_broker_mgr.hpp"
#include "kafka_topic_mgr.hpp"
#include "rdkafka.h"

class KafkaTopicMgrImpl:public KafkaTopicMgr
{
private:
    friend class KafkaTopicMgr;
public:
    KafkaTopicMgrImpl()
    {
        
    }
    
    ~KafkaTopicMgrImpl()
    {
      
    }

    std::string get_uuid_topic() override
    {
        return "test";
    }
    
    std::string get_service_topic() override
    {
        return "rpc_response";
    }
    
    std::string get_client_topic() override
    {
        return "rpc_request";
    }
};

class KafkaConfig
{
    KafkaConfig(std::string brokers)
    {
        rd_kafka_t *rk;         /* 基础句柄 */
        rd_kafka_conf_t *conf = NULL;  /* 配置结构体 */

        char errstr[512];
        /* Kafka configuration */
        if (NULL == conf) {
        conf = rd_kafka_conf_new();
        }

        rd_kafka_conf_set(conf, "queued.min.messages", "20", NULL, 0);
        rd_kafka_conf_set(conf, "bootstrap.servers", brokers.c_str(), errstr, sizeof(errstr));
        rd_kafka_conf_set(conf, "security.protocol", "sasl_plaintext", errstr, sizeof(errstr));
        rd_kafka_conf_set(conf, "sasl.mechanisms", "PLAIN", errstr, sizeof(errstr));

        //rd_kafka_conf_set(conf, "sasl.username", username.c_str(), errstr,sizeof(errstr));
        //rd_kafka_conf_set(conf, "sasl.password", password.c_str(), errstr,sizeof(errstr));
        rd_kafka_conf_set(conf, "api.version.request", "true", errstr, sizeof(errstr));

        /* Create Kafka handle */
        if (!(rk = rd_kafka_new(RD_KAFKA_PRODUCER, conf, errstr, sizeof(errstr)))) {
            fprintf(stderr, "%% Failed to init producer: %s\n", errstr);
            exit(1);
        }
    }

};

static KafkaTopicMgrImpl kafka_topic_instance_ = KafkaTopicMgrImpl();

KafkaTopicMgr * KafkaTopicMgr::instance()
{
    return &kafka_topic_instance_;
}
#endif


