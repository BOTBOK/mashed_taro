//
//  kafka_consumer.cpp
//  test_kafka
//
//  Created by 沈佳锋 on 2021/3/3.
//  Copyright © 2021 沈佳锋. All rights reserved.
//

#include "kafka_consumer.hpp"
#include <thread>
#include <vector>
#include "rdkafkacpp.h"
#include <iostream>
#include <unordered_map>
#include "kafka_broker_mgr.hpp"
#include <mutex>

static bool exit_eof = true;
static int eof_cnt = 0;
static int partition_cnt = 0;
static int verbosity = 1;

class ExampleEventCb : public RdKafka::EventCb {
public:
    void event_cb (RdKafka::Event &event) {
        switch (event.type())
        {
            case RdKafka::Event::EVENT_ERROR:
                std::cerr << "ERROR (" << RdKafka::err2str(event.err()) << "): " <<
                event.str() << std::endl;
                if (event.err() == RdKafka::ERR__ALL_BROKERS_DOWN)
                {}
                break;
                
            case RdKafka::Event::EVENT_STATS:
                std::cerr << "\"STATS\": " << event.str() << std::endl;
                break;
                
            case RdKafka::Event::EVENT_LOG:
                fprintf(stderr, "LOG-%i-%s: %s\n",
                        event.severity(), event.fac().c_str(), event.str().c_str());
                break;
                
            case RdKafka::Event::EVENT_THROTTLE:
                std::cerr << "THROTTLED: " << event.throttle_time() << "ms by " <<
                event.broker_name() << " id " << (int)event.broker_id() << std::endl;
                break;
                
            default:
                std::cerr << "EVENT " << event.type() <<
                " (" << RdKafka::err2str(event.err()) << "): " <<
                event.str() << std::endl;
                break;
        }
    }
};

class ConsumeCbBase: public RdKafka::ConsumeCb
{
private:
    long msg_cnt = 0;
    int64_t msg_bytes = 0;
    
protected:
    virtual void deal_msg_timed_out(RdKafka::Message &message)
    {
        std::cerr << "RdKafka::ERR__TIMED_OUT"<<std::endl;
    }
    
    virtual void deal_msg_no_error(RdKafka::Message &message)
    {
        /* Real message */
        msg_cnt++;
        msg_bytes += message.len();
        if (verbosity >= 3)
            std::cerr << "Read msg at offset " << message.offset() << std::endl;
        RdKafka::MessageTimestamp ts;
        ts = message.timestamp();
        if (verbosity >= 2 &&
            ts.type != RdKafka::MessageTimestamp::MSG_TIMESTAMP_NOT_AVAILABLE) {
            std::string tsname = "?";
            if (ts.type == RdKafka::MessageTimestamp::MSG_TIMESTAMP_CREATE_TIME)
                tsname = "create time";
            else if (ts.type == RdKafka::MessageTimestamp::MSG_TIMESTAMP_LOG_APPEND_TIME)
                tsname = "log append time";
            std::cout << "Timestamp: " << tsname << " " << ts.timestamp << std::endl;
        }
        if (verbosity >= 2 && message.key()) {
            std::cout << "Key: " << *message.key() << std::endl;
        }
        if (verbosity >= 1) {
            printf("%.*s\n",
                   static_cast<int>(message.len()),
                   static_cast<const char *>(message.payload()));
        }
    }
    
    virtual void deal_msg_partition_eof(RdKafka::Message &message)
    {
        /* Last message */
        if (exit_eof && ++eof_cnt == partition_cnt) {
            std::cerr << "%% EOF reached for all " << partition_cnt <<
            " partition(s)" << std::endl;
        }
    }
    
    virtual void deal_msg_unknow_topic(RdKafka::Message &message)
    {
        std::cerr << "Consume failed: " << message.errstr() << std::endl;
    }
    
    virtual void deal_msg_unknow_partition(RdKafka::Message &message)
    {
        std::cerr << "Consume failed: " << message.errstr() << std::endl;
    }
    
    virtual void deal_msg_unknow_error(RdKafka::Message &message)
    {
        /* Errors */
        std::cerr << "Consume failed: " << message.errstr() << std::endl;
    }
    
public:
    void consume_cb(RdKafka::Message &message, void* opaque) override{
        switch (message.err()) {
            case RdKafka::ERR__TIMED_OUT:
                deal_msg_timed_out(message);
                break;
            case RdKafka::ERR_NO_ERROR:
                deal_msg_no_error(message);
                break;
            case RdKafka::ERR__PARTITION_EOF:
                deal_msg_partition_eof(message);
                break;
            case RdKafka::ERR__UNKNOWN_TOPIC:
                deal_msg_unknow_topic(message);
                break;
            case RdKafka::ERR__UNKNOWN_PARTITION:
                deal_msg_unknow_partition(message);
                break;
            default:
                deal_msg_unknow_error(message);
        }
    }
};


class IKafkaConsumerImpl: public ConsumeCbBase,public KafkaConsumer
{
private:
    bool run_;
    
    //kafka全局配置
    RdKafka::Conf *conf_;
    
    //kafka topic配置
    RdKafka::Conf *tconf_;
    
    //kafka 消费者
    RdKafka::KafkaConsumer *consumer_;
    
    //thread
    std::thread *th_;
    
private:
    bool inited_;
    bool started_;
    bool released_;
    
private:
    std::mutex func_mutex_;
    typedef std::vector<ConsumerCbFunc> VecFunc;
    std::unordered_map<std::string, VecFunc> func_map_;
    
public:
    
    int subscribe_topic(std::string topic, ConsumerCbFunc func) override
    {
        //启动后不允许注册
        if(started_)
            return -1;
        
        std::lock_guard<std::mutex> lock(func_mutex_);
        auto _find = func_map_.find(topic);
        if(_find == func_map_.end())
        {
            VecFunc vec_func;
            vec_func.push_back(func);
            func_map_.emplace(topic, vec_func);
        }
        else
        {
            _find->second.push_back(func);
        }
        return 0;
    }
    
protected:
    void deal_msg_timed_out(RdKafka::Message &message) override
    {
        ConsumeCbBase::deal_msg_timed_out(message);
    }
    
    void deal_msg_no_error(RdKafka::Message &message) override
    {
        ConsumeCbBase::deal_msg_no_error(message);
        
        //当程序开始监听时 不允许注册，监听只有一个线程不做加锁处理
        auto _find = func_map_.find(message.topic()->name());
        if(_find != func_map_.end())
        {
            std::string data(static_cast<const char *>(message.payload()), message.len());
            for(auto item : _find->second)
            {
                item(data);
            }
        }
    }
    
    void deal_msg_partition_eof(RdKafka::Message &message) override
    {
        ConsumeCbBase::deal_msg_partition_eof(message);
        run_ = false;
    }
    
    void deal_msg_unknow_topic(RdKafka::Message &message) override
    {
        ConsumeCbBase::deal_msg_unknow_topic(message);
        run_ = false;
    }
    
    void deal_msg_unknow_partition(RdKafka::Message &message) override
    {
        ConsumeCbBase::deal_msg_unknow_partition(message);
        run_ = false;
    }
    
    void deal_msg_unknow_error(RdKafka::Message &message) override
    {
        /* Errors */
        ConsumeCbBase::deal_msg_unknow_error(message);
        run_ = false;
    }
    
private:
    void work_thread()
    {
        std::vector<std::string> topics;
        for(auto item : func_map_)
        {
            topics.push_back(item.first);
        }
        
        //订阅消费主题
        RdKafka::ErrorCode err = consumer_->subscribe(topics);
        if (err) {
            std::cerr << "Failed to subscribe to " << topics.size() << " topics: "
            << RdKafka::err2str(err) << std::endl;
            exit(1);
        }
        
        //循环消费
        while (run_) {
            //5000毫秒未订阅到消息，触发RdKafka::ERR__TIMED_OUT
            RdKafka::Message *msg = consumer_->consume(5000);
            consume_cb(*msg, NULL);
            delete msg;
        }
    }
    
public:
    IKafkaConsumerImpl():run_(true), conf_(nullptr),tconf_(nullptr), consumer_(nullptr), th_(nullptr), inited_(false), started_(false), released_(false)
    {}
    
    int init(std::string brokers, std::string group_id) override
    {
        if(inited_)
            return -1;
    
        conf_ = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
        tconf_ = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);
        
        std::string errstr;
        
        //设置消费组，同一消费组内，消息只能被消费一次
        if (conf_->set("group.id", group_id, errstr) != RdKafka::Conf::CONF_OK)
        {
            std::cerr << errstr << std::endl;
            return -1;
        }
        
        //集群信息 bootstrap.servers可以替换为metadata.broker.list， 设置brokers
        conf_->set("metadata.broker.list", brokers, errstr);
        
        //设置consume回调
        conf_->set("consume_cb", this, errstr);
        
        //设置event回调
        ExampleEventCb ex_event_cb;
        conf_->set("event_cb", &ex_event_cb, errstr);
        
        //默认topicz设置
        conf_->set("default_topic_conf", tconf_, errstr);
        
        //创建consumer
        consumer_ = RdKafka::KafkaConsumer::create(conf_, errstr);
        if (!consumer_) {
            std::cerr << "Failed to create consumer: " << errstr << std::endl;
            return -1;
        }
        std::cout << "% Created consumer " << consumer_->name() << std::endl;
        
        inited_ = true;
        return 0;
    }
    
    int start_consumer() override
    {
        if(!inited_ || started_ || released_)
            return -1;
        
        th_ = new std::thread(std::bind(&IKafkaConsumerImpl::work_thread, this));
        
        started_ = true;
        return 0;
    }
    
    int release() override
    {
        if (!inited_ || released_)
            return -1;
        
        run_ = false;
        if(th_)
        {
            th_->join();
            delete th_;
        }
        
        if(consumer_)
        {
            consumer_->close();
            delete consumer_;
        }
        
        if(conf_)
        {
            delete conf_;
        }
        
        if(tconf_)
        {
            delete tconf_;
        }
        
        //应用退出之前等待rdkafka清理资源
        RdKafka::wait_destroyed(5000);
        
        released_ = true;
        return 0;
    }
};

static void sigterm (int sig) {
    //todo
}

KafkaConsumer *create_kafka_consumer()
{
    return new IKafkaConsumerImpl();
}
