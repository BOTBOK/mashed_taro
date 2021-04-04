#include "kafka_broker_mgr.hpp"
#include "kafka_topic_mgr.hpp"

#include "kafka_rpc_client.hpp"
#include "kafka_rpc_exception.hpp"

#include <iostream>
#include <thread>

class ClientKafkaBrokerMgr: public KafkaBrokerMgr
{
public:
    std::string zk_list() override
    {
        return "127.0.0.1:2181";
    }
    
    std::string broker_list() override
    {
        return "127.0.0.1:9092";
    }
    
};


class ClientKafkaTopicMgr: public KafkaTopicMgr
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
        return "101";
    }
};


int main(int argc, char const *argv[])
{
    RPC_TRY
    {
        ClientKafkaBrokerMgr broker_mgr = ClientKafkaBrokerMgr();
        ClientKafkaTopicMgr topic_mgr = ClientKafkaTopicMgr();

        RpcClient *client_ = create_rpc_client(&broker_mgr, &topic_mgr);

        std::string response;
        std::cout << "request data:[hello config], call interfaceianme:[config]" << std::endl;
        client_->call("config", "hello config", response);
        std::cout << "response data:[" <<response <<"]" << std::endl;

    }
    RPC_CATCH
    {
        std::cout << "error: client_, err_code:[" << ex.err_code() << "], err_msg" << ex.err_msg() << std::endl;
    }



    return 0;
}





