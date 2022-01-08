#include"RpcComponent.h"
#include<Component/ServiceBase/ServiceComponent.h>
#include<Coroutine/TaskComponent.h>
#include<Util/StringHelper.h>
#include"Core/App.h"
#include<Pool/MessagePool.h>
#include<Method/LuaServiceMethod.h>
#include"Scene/RpcConfigComponent.h"
#include"Rpc/RpcClientComponent.h"
#include"Other/ElapsedTimer.h"
#include"Async/RpcTask/RpcTaskSource.h"
namespace GameKeeper
{
	bool RpcComponent::Awake()
	{
        this->mNodeId = 0;
        this->mCorComponent = nullptr;
        this->mTimerComponent = nullptr;
        this->mPpcConfigComponent = nullptr;
        this->mRpcClientComponent = nullptr;
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

	XCode RpcComponent::OnRequest(std::shared_ptr<com::Rpc_Request> request)
    {
        unsigned short methodId = request->methodid();
        const ProtoConfig *protocolConfig = this->mPpcConfigComponent->GetProtocolConfig(methodId);
        if (protocolConfig == nullptr)
        {
            return XCode::NotFoundRpcConfig;
        }

        const std::string &service = protocolConfig->Service;
        auto logicService = this->gameObject->GetComponent<ServiceComponent>(service);
        if (logicService == nullptr)
        {
            LOG_FATAL("call service not exist : [" << service << "]");
            return XCode::CallServiceNotFound;
        }

        if (!protocolConfig->IsAsync)
        {
            long long socketId = request->socketid();
            const std::string &method = protocolConfig->Method;
            auto response = logicService->Invoke(method, request);
            this->mRpcClientComponent->Send(socketId, response);
            return XCode::Successful;
        }
        this->mCorComponent->Start([request, this, logicService, protocolConfig]()
        {
            long long socketId = request->socketid();
            const std::string &method = protocolConfig->Method;
            auto response = logicService->Invoke(method, request);
            this->mRpcClientComponent->Send(socketId, response);
        });
        return XCode::Successful;
    }

    XCode RpcComponent::OnResponse(std::shared_ptr<com::Rpc_Response> response)
    {
        long long rpcId = response->rpcid();
        auto iter = this->mRpcTasks.find(rpcId);
        if(iter == this->mRpcTasks.end())
        {
            LOG_WARN("not find rpc task : " << rpcId)
            return XCode::Failure;
        }
        auto rpcTask = iter->second;
        this->mRpcTasks.erase(iter);
        rpcTask->OnResponse(response);
        return XCode::Successful;
    }

    void RpcComponent::AddRpcTask(std::shared_ptr<IRpcTask> task)
    {
        long long rpcId = task->GetRpcId();
        this->mRpcTasks.emplace(rpcId, task);
        if (task->GetTimeout() > 0)
        {
            this->mTimerComponent->AsyncWait(
                    task->GetTimeout(), &RpcComponent::OnTaskTimeout, this, rpcId);
        }
    }
#ifdef __DEBUG__
    void RpcComponent::AddRpcInfo(long long rpcId, int methodId)
    {
        RpcTaskInfo taskInfo;
        taskInfo.MethodId = methodId;
        taskInfo.Time = Helper::Time::GetMilTimestamp();
        this->mRpcInfoMap.emplace(rpcId, taskInfo);
    }

    bool RpcComponent::GetRpcInfo(long long int rpcId, int &methodId, long long int &time)
    {
        auto iter = this->mRpcInfoMap.find(rpcId);
        if(iter == this->mRpcInfoMap.end())
        {
            return false;
        }
        methodId = iter->second.MethodId;
        time = Helper::Time::GetMilTimestamp() - iter->second.Time;
        this->mRpcInfoMap.erase(iter);
        return true;
    }
#endif

    void RpcComponent::OnTaskTimeout(long long int rpcId)
    {
        auto iter = this->mRpcTasks.find(rpcId);
        if (iter != this->mRpcTasks.end())
        {
            auto rpcTask = iter->second;
            this->mRpcTasks.erase(iter);
            rpcTask->OnResponse(nullptr);
        }
#ifdef __DEBUG__
        int methodId = 0;
        long long costTime = 0;
        if(this->GetRpcInfo(rpcId, methodId, costTime))
        {
            auto config = this->mPpcConfigComponent->GetProtocolConfig(methodId);
            LOG_ERROR("call " << config->Service << "." << config->Method << " time out");
        }
#endif
    }
}// namespace GameKeeper
