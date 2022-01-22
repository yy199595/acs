#include"ServiceProxy.h"
#include"Object/App.h"
#include"Async/RpcTask/RpcTaskSource.h"
#include<Util/StringHelper.h>
#include<Scene/RpcConfigComponent.h>
#include"Rpc/RpcComponent.h"
#include"Service/NodeService.h"
namespace Sentry
{
	ServiceProxy::ServiceProxy(const std::string & name)
		: mServiceName(name)
	{
        this->mTaskComponent = App::Get().GetTaskComponent();
        this->mRpcComponent = App::Get().GetComponent<RpcComponent>();
        LOG_CHECK_RET(this->mRpcConfigComponent = App::Get().GetComponent<RpcConfigComponent>());
	}

    void ServiceProxy::AddAddress(const std::string &address)
    {
        auto iter = this->mServiceNodeMap.find(address);
        if(iter == this->mServiceNodeMap.end())
        {
            this->mAllAddress.emplace(std::move(address));
            auto node = make_shared<ProxyClient>
                    (this->mServiceName, address);
            this->mServiceNodeMap.emplace(address, node);
            LOG_WARN(this->mServiceName, " add new address [", address, "]");
        }
    }

    bool ServiceProxy::RemoveAddress(const std::string &address)
    {
        auto iter = this->mServiceNodeMap.find(address);
        if(iter != this->mServiceNodeMap.end())
        {
            this->mServiceNodeMap.erase(iter);
            NodeService * redisService = App::Get().GetComponent<NodeService>();
            return redisService != nullptr && redisService->RemoveNode(address);
        }
        return false;
    }

    std::shared_ptr<ProxyClient> ServiceProxy::GetNode(const std::string &address)
    {
        auto iter = this->mServiceNodeMap.find(address);
        return iter != this->mServiceNodeMap.end() ? iter->second : nullptr;
    }

    bool ServiceProxy::RemoveServiceNode(const std::string &address)
    {
        auto iter = this->mServiceNodeMap.find(address);
        if(iter == this->mServiceNodeMap.end())
        {
            auto serviceNode = iter->second;
            this->mServiceNodeMap.erase(iter);
            std::shared_ptr<com::Rpc_Request> message = serviceNode->PopMessage();
            while(message != nullptr)
            {
                int methodId = message->method_id();
                auto methodConfig = this->mRpcConfigComponent->GetProtocolConfig(methodId);
                if(methodConfig != nullptr)
                {
                    
                }
                message = serviceNode->PopMessage();
            }
        }
    }

    bool ServiceProxy::AllotServiceAddress(std::string &address)
    {
        if(this->mAllAddress.empty())
        {
            return false;
        }
        while(!this->mAllAddress.empty())
        {
            const std::string arr = this->mAllAddress.front();
            this->mAllAddress.pop();
            std::shared_ptr<ProxyClient> serviceNode = this->GetNode(arr);
            if(serviceNode != nullptr && serviceNode->IsConnected())
            {
                address = serviceNode->GetAddress();
                this->mAllAddress.emplace(address);
                return true;
            }
            this->RemoveAddress(std::move(arr));
        }
        return false;
    }

	std::shared_ptr<com::Rpc_Request> ServiceProxy::NewRequest(const std::string & method)
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

    XCode ServiceProxy::Call(const string &func, std::shared_ptr<RpcTaskSource> taskSource)
    {
        std::string address;
        if(this->AllotServiceAddress(address))
        {
            LOG_ERROR("allot service address failure : ", this->mServiceName);
            return XCode::CallServiceNotFound;
        }
        return this->Call(address, func, taskSource);
    }

    XCode ServiceProxy::Call(const string &func, const Message &message, std::shared_ptr<RpcTaskSource> taskSource)
    {
        std::string address;
        if (this->AllotServiceAddress(address))
        {
            LOG_ERROR("allot service address failure : ", this->mServiceName);
            return XCode::CallServiceNotFound;
        }
        return this->Call(address, func, message, taskSource);
    }

    XCode ServiceProxy::Call(const std::string &address, const std::string &func,
                             std::shared_ptr<RpcTaskSource> taskSource)
    {
        auto requestData = this->NewRequest(func);
        if (requestData == nullptr)
        {
            LOG_ERROR("{0} not config ", func);
            return XCode::NotFoundRpcConfig;
        }

        std::shared_ptr<ProxyClient> serviceNode = this->GetNode(address);
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

    XCode ServiceProxy::Call(const std::string &address, const std::string &func, const Message &message,
                             std::shared_ptr<RpcTaskSource> taskSource)
    {
        auto requestData = this->NewRequest(func);
        if (requestData == nullptr)
        {
            LOG_ERROR("{0} not rpc config ", func);
            return XCode::NotFoundRpcConfig;
        }
        std::shared_ptr<ProxyClient> serviceNode = this->GetNode(address);
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
}// namespace Sentry