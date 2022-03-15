#include"ServiceProxy.h"
#include"Object/App.h"
#include"Async/RpcTask/RpcTaskSource.h"
#include<Util/StringHelper.h>
#include<Scene/RpcConfigComponent.h>
#include"Rpc/RpcComponent.h"
#include"Service/LocalService.h"
#include"DB/Component/RedisComponent.h"
namespace Sentry
{
	ServiceProxy::ServiceProxy(const std::string & name)
		: mServiceName(name), mApp(App::Get())
	{
        this->mIndex = 0;
        this->mTaskComponent = this->mApp.GetTaskComponent();
        this->mTimerComponent = this->mApp.GetTimerComponent();
        this->mRpcComponent = this->mApp.GetComponent<RpcComponent>();
        LOG_CHECK_RET(this->mRpcConfigComponent = this->mApp.GetComponent<RpcConfigComponent>());
	}

    void ServiceProxy::AddAddress(const std::string &address)
    {
        auto iter = this->mServiceNodeMap.find(address);
        if(iter == this->mServiceNodeMap.end())
        {
            this->mAllAddress.emplace_back(address);
            auto node = make_shared<ProxyClient>
                    (this->mServiceName, address);
            this->mServiceNodeMap.emplace(address, node);
            while(!this->mWaitTaskQueue.empty())
            {
                std::shared_ptr<TaskSource<bool>> taskSource = this->mWaitTaskQueue.front();
                if(taskSource != nullptr)
                {
                    taskSource->SetResult(false);
                }
                this->mWaitTaskQueue.pop();
            }
            LOG_WARN(this->mServiceName, " add new address [", address, "]");
        }
    }

    bool ServiceProxy::RemoveAddress(const std::string &address)
    {
        auto iter = this->mServiceNodeMap.find(address);
        if(iter != this->mServiceNodeMap.end())
        {
            this->mServiceNodeMap.erase(iter);
            return true;
        }
        return false;
    }

    std::shared_ptr<ProxyClient> ServiceProxy::GetNode(const std::string &address)
    {
        auto iter = this->mServiceNodeMap.find(address);
        return iter != this->mServiceNodeMap.end() ? iter->second : nullptr;
    }

    std::string ServiceProxy::AllotAddress(int ms)
    {
        if (this->mAllAddress.empty())
        {
            std::shared_ptr<TaskSource<bool>> taskSource(new TaskSource<bool>());
            auto OnTimerCallback = [taskSource]() {
                if (!taskSource->IsComplete())
                {
                    taskSource->SetResult(false);
                }
            };

            unsigned int timerId = this->mTimerComponent->AddTimer(
                    ms, new LambdaMethod(std::move(OnTimerCallback)));
            this->mWaitTaskQueue.emplace(taskSource);
#ifdef __DEBUG__
            LOG_WARN("wait new service");
#endif
            if (!taskSource->Await())
            {
                return std::string();
            }
			this->mTimerComponent->CancelTimer(timerId);
        }

        if (this->mIndex >= this->mAllAddress.size())
        {
            this->mIndex = 0;
        }

        const std::string &address = this->mAllAddress[this->mIndex];
        std::shared_ptr<ProxyClient> serviceNode = this->GetNode(address);
        if (serviceNode != nullptr && serviceNode->IsConnected())
        {
            this->mIndex++;
            return address;
        }
        LocalService *localService = this->mApp.GetComponent<LocalService>();
        if (localService != nullptr)
        {
            localService->RemoveByAddress(address);
            this->mAllAddress.erase(this->mAllAddress.begin() + this->mIndex);
        }
        return std::string();
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

    std::shared_ptr<RpcTaskSource> ServiceProxy::Call(const string &func)
    {
        std::string address = this->AllotAddress();
        if(address.empty())
        {
            LOG_ERROR("allot service address failure : ", this->mServiceName);
            return nullptr;
        }
        return this->Call(address, func);
    }

    std::shared_ptr<RpcTaskSource> ServiceProxy::Call(const string &func, const Message &message)
    {
        std::string address = this->AllotAddress();
        if (address.empty())
        {
            LOG_ERROR("allot service address failure : ", this->mServiceName);
            return nullptr;
        }
        return this->Call(address, func, message);
    }

    std::shared_ptr<RpcTaskSource> ServiceProxy::Call(const std::string &address, const std::string &func)
    {
        auto requestData = this->NewRequest(func);
        if (requestData == nullptr) {
            LOG_ERROR("{0} not config ", func);
            return nullptr;
        }

        std::shared_ptr<ProxyClient> serviceNode = this->GetNode(address);
        if (serviceNode == nullptr) {
            LOG_ERROR("not find node : ", address);
            return nullptr;
        }
        std::shared_ptr<RpcTaskSource> taskSource(new RpcTaskSource());
        this->mRpcComponent->AddRpcTask(taskSource);
        requestData->set_rpc_id(taskSource->GetRpcId());
#ifdef __DEBUG__
        this->mRpcComponent->AddRpcInfo(taskSource->GetRpcId(), requestData->method_id());
#endif
        serviceNode->PushMessage(requestData);
        return taskSource;
    }

    std::shared_ptr<RpcTaskSource> ServiceProxy::Call(const std::string &address, const std::string &func, const Message &message)
    {
        auto requestData = this->NewRequest(func);
        if (requestData == nullptr) {
            LOG_ERROR("{0} not rpc config ", func);
            return nullptr;
        }
        std::shared_ptr<ProxyClient> serviceNode = this->GetNode(address);
        if (serviceNode == nullptr) {
            LOG_ERROR("not find node : ", address);
            return nullptr;
        }
        requestData->mutable_data()->PackFrom(message);
        std::shared_ptr<RpcTaskSource> taskSource(new RpcTaskSource());

        this->mRpcComponent->AddRpcTask(taskSource);
        requestData->set_rpc_id(taskSource->GetRpcId());
#ifdef __DEBUG__
        this->mRpcComponent->AddRpcInfo(taskSource->GetRpcId(), requestData->method_id());
#endif
        serviceNode->PushMessage(requestData);
        return taskSource;
    }
}// namespace Sentry