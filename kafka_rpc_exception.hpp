//
//  rpc_exception.hpp
//  test_kafka
//
//  Created by 沈佳锋 on 2021/3/16.
//  Copyright © 2021 沈佳锋. All rights reserved.
//

#ifndef rpc_exception_hpp
#define rpc_exception_hpp

#include <stdio.h>
#include <exception>
#include <string>
#include <sstream>

class RpcException : public std::exception
{
public:
    virtual const char* what() const noexcept override
    {
        return what_msg_.c_str();
    }
    
    RpcException(int32_t err_code, std::string err_msg, std::string brokers = "", std::string topics = ""):brokers_(brokers), topics_(topics), err_code_(err_code), err_msg_(err_msg)
    {
        std::stringstream ss;
        ss << "brokers:[" << brokers_ << "],topics:[" << topics_ << "],err_code:[" << err_code_ << "],err_msg:[" << err_msg_ << "].";
        what_msg_ = ss.str();
    }
    
     RpcException(const RpcException &copy)
     {
         brokers_ = copy.brokers_;
         topics_ = copy.topics_;
         err_code_ = copy.err_code_;
         err_msg_ = copy.err_msg_;
         
         what_msg_ = copy.what_msg_;
     }
    
    std::string brokers()
    {
        return brokers_;
    }
    
    std::string topics()
    {
        return topics_;
    }
    
    int32_t err_code()
    {
        return err_code_;
    }
    
    std::string err_msg()
    {
        return err_msg_;
    }
 
private:
    std::string brokers_;
    std::string topics_;
    int32_t err_code_;
    std::string err_msg_;
    
    std::string what_msg_;
};

#define RPC_TRY  try { try

#define RPC_CATCH   catch (RpcException &e){\
          throw RpcException(e);\
      }\
      catch (std::exception &e) {\
          throw RpcException(-1, e.what());\
      }\
      catch (...)\
      {\
          throw RpcException(-1, "unknow error");\
      }\
  }catch(RpcException &ex)

#define RPC_THROW(err_code, err_message) {std::stringstream ss; \
    ss << err_message;\
    throw RpcException(err_code, ss.str());}
#define RPC_THROW_IF(condition, err_code, err_message) if(condition) {std::stringstream ss; \
    ss << err_message;\
    throw RpcException(err_code, ss.str());}
/**
int test()
 {
     RPC_TRY{
         printf("");
         RPC_THROW(-1, "hello");
     }
     RPC_CATCH{
         printf("%s", ex.what());
     }
     return 0;
 }
 */


#endif /* rpc_exception_hpp */
