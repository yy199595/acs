#include"ServiceEntity.h"
#include<Core/App.h>
#include"Async/RpcTask/RpcTaskSource.h"
#include<Util/StringHelper.h>
#include<Scene/RpcConfigComponent.h>
#include"Rpc/RpcComponent.h"
#include"Service/RedisService.h"
namespace GameKeeper
{
	ServiceEntity::ServiceEntity(const std::string & name)
		: mServiceName(name)
	{
        this->mTaskComponent = App::Get().GetTaskComponent();
        this->mRpcComponent = App::Get().GetComponent<RpcComponent>();
        LOG_CHECK_RET(this->mRpcConfigComponent = App::Get().GetComponent<RpcConfigComponent>());
	}

    void ServiceEntity::AddAddress(const std::string &address)
    {
        auto iter = this->mServiceNodeMap.find(address);
        if(iter == this->mServiceNodeMap.end())
        {
            auto node = make_shared<ServiceNode>
                    (this->mServiceName, address);
            this->mServiceNodeMap.emplace(address, node);
            LOG_WARN(this->mServiceName, " add new address ", address);
        }
    }

    bool ServiceEntity::RemoveAddress(const std::string &address)
    {
        auto iter = this->mServiceNodeMap.find(address);
        if(iter != this->mServiceNodeMap.end())
        {
            this->mServiceNodeMap.erase(iter);
            return true;
        }
        return false;
    }

    std::shared_ptr<ServiceNode> ServiceEntity::GetNode(const std::string &address)
    {
        auto iter = this->mServiceNodeMap.find(address);
        return iter != this->mServiceNodeMap.end() ? iter->second : nullptr;
    }

    //从redis查询服务地址
    bool ServiceEntity::AllotServiceAddress(std::string &address)
    {
        if(this->mTaskComponent->GetContextId() != 0)
        {
           RedisService * redisService = App::Get().GetComponent<RedisService>();
           if(redisService == nullptr)
           {
               return false;
           }
           std::vector<std::string> nodes = redisService->QueryService(this->mServiceName);
           for(const std::string & address : nodes)
           {
               this->AddAddress(address);
           }
        }
        if(this->mServiceNodeMap.empty())
        {
            return false;
        }
        auto iter = this->mServiceNodeMap.begin();
        for(; iter != this->mServiceNodeMap.end(); iter ++)
        {
            address = iter->first;
            return true;
        }
        return false;
    }

	std::shared_ptr<com::Rpc_Request> ServiceEntity::NewRequest(const std::string & method)
	{
		auto config = this->mRpcConfigComponent->GetProtocolConfig(method);
		if (config == nullptr)
		{
			return nullptr;
		}
        std::shared_ptr<com::Rpc_Request> request(new com::Rpc_Request());
		request->set_method_id(config->MethodId);
        return request;
	}

    XCode ServiceEntity::Call(const string &func, std::shared_ptr<RpcTaskSource> taskSource)
    {
        std::string address;
        if(this->AllotServiceAddress(address))
        {
            LOG_ERROR("allot service address failure : ", this->mServiceName);
            return XCode::CallServiceNotFound;
        }
        return this->Call(address, func, taskSource);
    }

    XCode ServiceEntity::Call(const string &func, const Message &message, std::shared_ptr<RpcTaskSource> taskSource)
    {
        std::string address;
        if (this->AllotServiceAddress(address))
        {
            LOG_ERROR("allot service address failure : ", this->mServiceName);
            return XCode::CallServiceNotFound;
        }
        return this->Call(address, func, message, taskSource);
    }

    XCode ServiceEntity::Call(const std::string &address, const std::string &func,
                              std::shared_ptr<RpcTaskSource> taskSource)
    {
        auto requestData = this->NewRequest(func);
        if (requestData == nullptr)
        {
            LOG_ERROR("{0} not config ", func);
            return XCode::NotFoundRpcConfig;
        }

        std::shared_ptr<ServiceNode> serviceNode = this->GetNode(address);
        if(serviceNode == nullptr)
        {
            LOG_ERROR("not find node : ", address);
            return XCode::CallServiceNotFound;
        }
        if(taskSource != nullptr)
        {
            this->mRpcComponent->AddRpcTask(taskSource);
            requestData->set_rpc_id(taskSource->GetRpcId());
#ifdef __DEBUG__
            this->mRpcComponent->AddRpcInfo(taskSource->GetRpcId(), requestData->method_id());
#endif
            serviceNode->PushMessage(requestData);
            return taskSource->GetCode();
        }
        serviceNode->PushMessage(requestData);
        return XCode::Successful;
    }

    XCode ServiceEntity::Call(const std::string &address, const std::string &func, const Message &message,
                              std::shared_ptr<RpcTaskSource> taskSource)
    {
        auto requestData = this->NewRequest(func);
        if (requestData == nullptr)
        {
            LOG_ERROR("{0} not rpc config ", func);
            return XCode::NotFoundRpcConfig;
        }
        std::shared_ptr<ServiceNode> serviceNode = this->GetNode(address);
        if(serviceNode == nullptr)
        {
            LOG_ERROR("not find node : ", address);
            return XCode::CallServiceNotFound;
        }
        requestData->mutable_data()->PackFrom(message);
        if(taskSource != nullptr)
        {
            this->mRpcComponent->AddRpcTask(taskSource);
            requestData->set_rpc_id(taskSource->GetRpcId());
#ifdef __DEBUG__
            this->mRpcComponent->AddRpcInfo(taskSource->GetRpcId(), requestData->method_id());
#endif
            serviceNode->PushMessage(requestData);
            return taskSource->GetCode();
        }
        serviceNode->PushMessage(requestData);
        return XCode::Successful;
    }
}// namespace GameKeeper