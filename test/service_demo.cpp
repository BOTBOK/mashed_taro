#include "kafka_broker_mgr.hpp"
#include "kafka_topic_mgr.hpp"

#include "kafka_rpc_server.hpp"
#include "kafka_rpc_exception.hpp"

#include <iostream>

class ServiceKafkaBrokerMgr: public KafkaBrokerMgr
{
public:
    std::string zk_list() override
    {
        return "10.50.38.233:2181,10.50.38.234:2181,10.50.38.235:2181";
    }
    
    std::string broker_list() override
    {
        return "10.50.38.233:9092,10.50.38.234:9092,10.50.38.235:9092";
    }
    
};


class ServiceKafkaTopicMgr: public KafkaTopicMgr
{
public:
    std::string client_topic() 
    {
    	return "client-test";
    }

    std::string client_group_id()
    {
    	return "100";
    }

    std::string service_topic()
    {
    	return "service-test";
    }

    std::string service_group_id()
    {
        return "100";
    }
};


/**
 *@brief rpc回调借口
 */
class InterfaceCfonfig: public RpcInterface
{
public:
    /**
     *@brief 接口名称
     *@return 返回接口名称
     */
    std::string interface_name() override
    {
        return "config";
    }
    
    /**
     * @brief 服务端rpc回调函数，服务端异常支持抛rpc_exception和返回值两种类型
     * @parameter request
     * @parameter response
     * @return 成功返回0，失败返回-1
     */
    int rpc_service_cb(const std::string &request, std::string &response) override
    {
        std::cout << "receive message:" << request << std::endl;

        response = "InterfaceCfonfig response";

        std::cout << "response message:" << response << std::endl; 
        return 0;
    }
};

int main(int argc, char const *argv[])
{
    RPC_TRY
    {
        ServiceKafkaBrokerMgr broker_mgr = ServiceKafkaBrokerMgr();
        ServiceKafkaTopicMgr topic_mgr = ServiceKafkaTopicMgr();

        RpcService *service_ = create_rpc_service(&broker_mgr, &topic_mgr);

        InterfaceCfonfig interface_cfg_;
        service_->register_service_cb(&interface_cfg_);

        service_->start();
    }
    RPC_CATCH
    {
        std::cout << "error: rpc_service, err_code:[" << ex.err_code() << "], err_msg" << ex.err_msg() << std::endl;
    }

    return 0;
}





