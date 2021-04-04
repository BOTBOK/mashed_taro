//
//  kafka_product.cpp
//  test_kafka
//
//  Created by 沈佳锋 on 2021/3/4.
//  Copyright © 2021 沈佳锋. All rights reserved.
//

#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <csignal>
#include <cstring>

#include "kafka_producter.hpp"

#include "rdkafkacpp.h"



static volatile sig_atomic_t run = 1;

static void sigterm (int sig) {
    run = 0;
}

class ExampleDeliveryReportCb : public RdKafka::DeliveryReportCb {
public:
    void dr_cb (RdKafka::Message &message) {
        /* If message.err() is non-zero the message delivery failed permanently
         * for the message. */
        if (message.err())
            std::cerr << "% Message delivery failed: " << message.errstr() << std::endl;
        else
            std::cerr << "% Message delivered to topic " << message.topic_name() <<
            " [" << message.partition() << "] at offset " <<
            message.offset() << std::endl;
    }
};

class KakfaProducterImpl: public KafkaProducter
{
private:
    std::string brokers_;
    
    //kafka 配置
    RdKafka::Conf *conf_;
    
    //kafka 生产者
    RdKafka::Producer *producer_;
    
private:
    bool inited_;
    bool released_;
    
public:
    int init(std::string brokers) override
    {
        if(inited_)
            return -1;
        
        conf_ = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
        
        //
        std::string errstr;
        
        if (conf_->set("bootstrap.servers", brokers, errstr) != RdKafka::Conf::CONF_OK) {
            std::cerr << errstr << std::endl;
            return -1;
        }
        
        ExampleDeliveryReportCb ex_dr_cb;
        if (conf_->set("dr_cb", &ex_dr_cb, errstr) != RdKafka::Conf::CONF_OK) {
            std::cerr << errstr << std::endl;
            return -1;
        }
        
        producer_ = RdKafka::Producer::create(conf_, errstr);
        if (!producer_) {
            std::cerr << "Failed to create producer: " << errstr << std::endl;
            return -1;
        }
        
        inited_ = true;
        return 0;
    }

      
    int release() override
    {
        if(released_ || !inited_)
            return -1;
        
        if(conf_ != nullptr)
            delete conf_;
               
        if(producer_ != nullptr)
        {
           /* Wait for final messages to be delivered or fail.
            * flush() is an abstraction over poll() which
            * waits for all messages to be delivered. */
           std::cerr << "% Flushing final messages..." << std::endl;
           producer_->flush(10*1000 /* wait for max 10 seconds */);
           
           if (producer_->outq_len() > 0)
               std::cerr << "% " << producer_->outq_len() <<
               " message(s) were not delivered" << std::endl;
           
           delete producer_;
        }
        
        released_ = true;
        return 0;
    }
    
    int product_message(const std::string &topic, const std::string &data) override
    {
        if(!inited_ || released_)
            return -1;
        RdKafka::ErrorCode err =
        producer_->produce(
                           /* Topic name */
                           topic.c_str(),
                           /* Any Partition: the builtin partitioner will be
                            * used to assign the message to a topic based
                            * on the message key, or random partition if
                            * the key is not set. */
                           RdKafka::Topic::PARTITION_UA,
                           /* Make a copy of the value */
                           RdKafka::Producer::RK_MSG_COPY /* Copy payload */,
                           /* Value */
                           (void*)(data.c_str()),data.size(),
                           /* Key */
                           NULL, 0,
                           /* Timestamp (defaults to current time) */
                           0,
                           /* Message headers, if any */
                           NULL,
                           /* Per-message opaque value passed to
                            * delivery report */
                           NULL);
        
        if (err != RdKafka::ERR_NO_ERROR) {
            std::cerr << "% Failed to produce to topic " << topic << ": " <<
            RdKafka::err2str(err) << std::endl;
            
            if (err == RdKafka::ERR__QUEUE_FULL) {
                /* If the internal queue is full, wait for
                 * messages to be delivered and then retry.
                 * The internal queue represents both
                 * messages to be sent and messages that have
                 * been sent or failed, awaiting their
                 * delivery report callback to be called.
                 *
                 * The internal queue is limited by the
                 * configuration property
                 * queue.buffering.max.messages */
                producer_->poll(1000/*block for max 1000ms*/);
                return -1;
            }
            
        } else {
            std::cerr << "% Enqueued message (" << data.size() << " bytes) " <<
            "for topic " << topic << std::endl;
        }
        
        /* A producer application should continually serve
         * the delivery report queue by calling poll()
         * at frequent intervals.
         * Either put the poll call in your main loop, or in a
         * dedicated thread, or call it after every produce() call.
         * Just make sure that poll() is still called
         * during periods where you are not producing any messages
         * to make sure previously produced messages have their
         * delivery report callback served (and any other callbacks
         * you register). */
        producer_->poll(0);
        
        
        return 0;
    }
    
    KakfaProducterImpl():conf_(nullptr), inited_(false), released_(false){}
    
    ~KakfaProducterImpl(){}
};

KafkaProducter* create_kafka_producter()
{
    return new KakfaProducterImpl();
}
