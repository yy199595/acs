#include"RpcComponent.h"
#include<Service/ServiceComponent.h>
#include<Coroutine/TaskComponent.h>
#include<Util/StringHelper.h>
#include"Core/App.h"
#include<Pool/MessagePool.h>
#include<Method/LuaServiceMethod.h>
#include"Scene/RpcConfigComponent.h"
#include"ServerRpc/RpcClientComponent.h"
#include"Other/ElapsedTimer.h"
#include"Async/RpcTask/RpcTask.h"
namespace GameKeeper
{
	bool RpcComponent::Awake()
	{
        this->mTick = 1;
        this->mNodeId = 0;
        this->mCorComponent = nullptr;
        this->mTimerComponent = nullptr;
        this->mPpcConfigComponent = nullptr;
        this->mRpcClientComponent = nullptr;
        this->mLastTime = Helper::Time::GetSecTimeStamp();
		const ServerConfig & ServerCfg = App::Get().GetConfig();
		LOG_CHECK_RET_FALSE(ServerCfg.GetValue("NodeId", this->mNodeId));
        return true;
	}

    bool RpcComponent::LateAwake()
    {
        this->mCorComponent = App::Get().GetTaskComponent();
        this->mTimerComponent = this->GetComponent<TimerComponent>();
        LOG_CHECK_RET_FALSE(this->mCorComponent = this->GetComponent<TaskComponent>());
        LOG_CHECK_RET_FALSE(this->mPpcConfigComponent = this->GetComponent<RpcConfigComponent>());
        LOG_CHECK_RET_FALSE(this->mRpcClientComponent = this->GetComponent<RpcClientComponent>());
        return true;
    }

	bool RpcComponent::OnRequest(const com::Rpc_Request * request)
    {
        unsigned short methodId = request->methodid();
        const ProtocolConfig *protocolConfig = this->mPpcConfigComponent->GetProtocolConfig(methodId);
        if (protocolConfig == nullptr) {
            return false;
        }

        const std::string &service = protocolConfig->Service;
        auto logicService = this->gameObject->GetComponent<ServiceComponent>(service);
        if (logicService == nullptr) {
            LOG_FATAL("call service not exist : [" << service << "]");
            return false;
        }

        if(!protocolConfig->IsAsync)
        {
            long long socketId = request->socketid();
            const std::string &method = protocolConfig->Method;
            auto response = logicService->Invoke(method, request);
            this->mRpcClientComponent->SendByAddress(socketId, response);
            return true;
        }
        else {
            this->mCorComponent->Start([request, this, logicService, protocolConfig]() {
                long long socketId = request->socketid();
                const std::string &method = protocolConfig->Method;
                auto response = logicService->Invoke(method, request);
                this->mRpcClientComponent->SendByAddress(socketId, response);
            });
            return true;
        }
    }

    bool RpcComponent::OnResponse(const com::Rpc_Response *response)
    {
        LocalObject<com::Rpc_Response> local(response);
        long long rpcId = response->rpcid();
        auto iter = this->mRpcTasks.find(rpcId);
        if(iter == this->mRpcTasks.end())
        {
            LOG_WARN("not find rpc task : " << rpcId)
            return false;
        }
        auto rpcTask = iter->second;
        this->mRpcTasks.erase(iter);
        rpcTask->SetResult(response);
        return true;
    }

    unsigned int RpcComponent::AddRpcTask(std::shared_ptr<RpcTaskBase> task)
    {
        int methodId = task->GetMethodId();
        long long rpcId = task->GetTaskId();
        this->mRpcTasks.emplace(rpcId, task);
        auto config = this->mPpcConfigComponent->GetProtocolConfig(methodId);
        if (config != nullptr && config->Timeout > 0)
        {
            return this->mTimerComponent->AsyncWait(
                    config->Timeout, &RpcComponent::OnTaskTimeout, this, rpcId);
        }
        return 0;
    }

    std::shared_ptr<RpcTaskBase> RpcComponent::GetRpcTask(long long int rpcId) const
    {
        auto iter = this->mRpcTasks.find(rpcId);
        return iter != this->mRpcTasks.end() ? iter->second : nullptr;
    }

    void RpcComponent::OnTaskTimeout(long long int rpcId)
    {
        auto iter = this->mRpcTasks.find(rpcId);
        if (iter != this->mRpcTasks.end())
        {
            auto rpcTask = iter->second;
            this->mRpcTasks.erase(iter);
            rpcTask->SetResult(nullptr);
        }
    }
}// namespace GameKeeper
