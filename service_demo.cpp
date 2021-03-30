#include "kafka_broker_mgr.hpp"
#include "kafka_topic_mgr.hpp"

#include "kafka_rpc_server.hpp"



class ServiceKafkaBrokerMgr: public KafkaBrokerMgr
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


class ServiceKafkaTopicMgr: public KafkaTopicMgr
{
public:
    std::string client_topic() 
    {
    	return "client-test";
    }

    std::string client_group_id()
    {
    	return "100"
    }

    std::string service_topic()
    {
    	
    }

    std::string service_group_id() = 0;
    
};







