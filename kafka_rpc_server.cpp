
#include "kafka_rpc_server.hpp"

#include <string>
#include <thread>
#include <vector>
#include <unordered_map>

#include "kafka_rpc_message.hpp"
#include "kafka_producter.hpp"
#include "kafka_consumer.hpp"
#include "kafka_topic_mgr.hpp"
#include "kafka_rpc_exception.hpp"
#include "kafka_broker_mgr.hpp"
#include <mutex>
#include <condition_variable>
#include "kafka_broker_mgr.hpp"

class RpcServiceImpl: public RpcService
{
private:
    KafkaProducter *kafka_producter_;
    KafkaConsumer *kafka_cunsumer_;
    
    std::mutex data_mutex_;
    std::vector<std::string> data_list_;
    
    std::thread *th_;
    
    std::mutex rpc_cb_mutex_;
    std::unordered_map<std::string, RpcInterface*> rpc_cb_map_;
    
    std::condition_variable cond_;

    std::string brokers_;
    std::string service_topic_;
    
public:
    void deal_message()
    {
        int32_t err_num = 0;
        
        // 测试代码 单线程
        while(true)
        {
            RPC_TRY
            {
                std::vector<std::string> tmp_data_list;
                {
                    std::unique_lock<std::mutex> lock(data_mutex_);
                    if(data_list_.empty())
                        cond_.wait(lock);
                    tmp_data_list = data_list_;
                    data_list_.clear();
                }
                
                for(auto item : tmp_data_list)
                {
                    RpcMessage msg;
                    msg.from_string(item);
                    RpcInterface* cb = nullptr;
                    {
                        std::lock_guard<std::mutex> lock(rpc_cb_mutex_);
                        auto _find = rpc_cb_map_.find(msg.interface_name());
                        if(_find != rpc_cb_map_.end())
                            cb = _find->second;
                    }
                    
                    if(nullptr == cb)
                        continue;

                    std::string response;
                    //rpc try catch 可以捕获所有异常，防止rpc调用代码调用出错抛出异常
                    RPC_TRY
                    {
                        int rpc_ret = cb->rpc_service_cb(msg.data(), response);
                        if(rpc_ret != 0)
                        {
                            printf("rpc_call fail, err_code:[%d]", rpc_ret);
                        }
                    }
                    RPC_CATCH
                    {
                        //todo error
                        printf("rpc call error errcode:[%d] errmsg:[%s]", ex.err_code(), ex.err_msg().c_str());
                    }
                    
                    RpcMessage response_msg;
                    response_msg.set_data(response);
                    response_msg.set_interface_name(response_msg.interface_name());
                    response_msg.set_response_topic(msg.response_topic());
                    kafka_producter_->product_message(msg.response_topic(), response_msg.to_string());
                }
                
                //异常次数清零
                err_num = 0;
            }
            RPC_CATCH
            {
                //todo error
                printf("errcode:[%d] errmsg:[%s]", ex.err_code(), ex.err_msg().c_str());
                
                //异常情况则sleep 100ms,防止一直异常，cpu空转
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                
                //10分钟之内都是异常 太无聊了 不干了
                if(++err_num > 10 * 60 * 60 * 10)
                    break;
            }
        }
    }
    
    void get_message(std::string &data)
    {
        {
            std::lock_guard<std::mutex> lock(data_mutex_);
            data_list_.push_back(data);
        }
        cond_.notify_all();
    }
    
    RpcServiceImpl(KafkaBrokerMgr* broker_mgr, KafkaTopicMgr* topic_mgr):kafka_producter_(nullptr), kafka_cunsumer_(nullptr), th_(nullptr), 
        brokers_(broker_mgr->broker_list()), service_topic_(topic_mgr->service_topic())
    {
        kafka_producter_ = create_kafka_producter();
        kafka_cunsumer_ = create_kafka_consumer();
        
        //创建不成功 则直接抛出异常
        RPC_THROW_IF(kafka_producter_ == nullptr, -1, "create_kafka_producter error");
        RPC_THROW_IF(kafka_cunsumer_ == nullptr, -1, "create_kafka_consumer error");
        
        //broker和消费组
        kafka_cunsumer_->init(broker_mgr->broker_list(), topic_mgr->service_group_id());

        //初始化producter
        kafka_producter_->init(broker_mgr->broker_list());

        //设置consumer topic
        kafka_cunsumer_->subscribe_topic(topic_mgr->service_topic(), std::bind(&RpcServiceImpl::get_message, this, std::placeholders::_1));
        
        th_ = new std::thread(std::bind(&RpcServiceImpl::deal_message, this));
        RPC_THROW_IF(th_ == nullptr, -1, "create thread error");
    }
    
    ~RpcServiceImpl()
    {
        if(kafka_producter_ != nullptr)
            delete kafka_producter_;
        if(th_ != nullptr)
            delete th_;
    }
    
    void register_service_cb(RpcInterface *rpc_cb) override
    {
        std::lock_guard<std::mutex> lock(rpc_cb_mutex_);
        
        if(rpc_cb_map_.find(rpc_cb->interface_name()) != rpc_cb_map_.end())
        {
            RPC_THROW(RPC_SERVICE_REGISTER_ERROR, "接口：" << rpc_cb->interface_name() << "已注册");
        }
            
        RPC_THROW_IF(!rpc_cb_map_.emplace(rpc_cb->interface_name(), rpc_cb).second, RPC_SERVICE_REGISTER_ERROR, "注册失败！");
    }

    void start() override
    {
        RPC_THROW_IF(0!= kafka_cunsumer_->start_consumer(), -1, "start_consumer error!");
    }
};

RpcService * create_rpc_service(KafkaBrokerMgr* broker_mgr, KafkaTopicMgr* topic_mgr)
{
     return new RpcServiceImpl(broker_mgr, topic_mgr);
}
