#include "ProtoRpcComponent.h"
#include <Service/ProtoServiceComponent.h>
#include <Coroutine/TaskComponent.h>
#include <Util/StringHelper.h>
#include"Core/App.h"
#include <Pool/MessagePool.h>
#include <Method/LuaServiceMethod.h>
#include"Scene/RpcConfigComponent.h"
#include"ServerRpc/ProtoRpcClientComponent.h"
#include "Other/ElapsedTimer.h"
#include"Async/RpcTask/ProtoRpcTask.h"
namespace GameKeeper
{
	bool ProtoRpcComponent::Awake()
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

    bool ProtoRpcComponent::LateAwake()
    {
        this->mCorComponent = App::Get().GetTaskComponent();
        this->mTimerComponent = this->GetComponent<TimerComponent>();
        LOG_CHECK_RET_FALSE(this->mCorComponent = this->GetComponent<TaskComponent>());
        LOG_CHECK_RET_FALSE(this->mPpcConfigComponent = this->GetComponent<RpcConfigComponent>());
        LOG_CHECK_RET_FALSE(this->mRpcClientComponent = this->GetComponent<ProtoRpcClientComponent>());
        return true;
    }

	bool ProtoRpcComponent::OnRequest(const com::Rpc_Request * request)
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

        const std::string &methodName = protocolConfig->Method;
        ServiceMethod *method = logicService->GetProtoMethod(methodName);
        if (method == nullptr) {
            LOG_FATAL("call method not exist : [" << service << "." << methodName << "]");
            return false;
        }

        if (!protocolConfig->Request.empty() && !request->has_data())
        {
            LocalObject<com::Rpc_Request> lock(request);
            if (request->rpcid() != 0)
            {
                auto response = new com::Rpc_Response();
                response->set_rpcid(request->rpcid());
                response->set_userid(request->userid());
                response->set_code(XCode::CallArgsError);
                this->mRpcClientComponent->SendByAddress(request->socketid(), response);
            }
            return true;
        }

        if (!protocolConfig->IsAsync) {
#ifdef __DEBUG__
            ElapsedTimer elapsedTimer;
#endif
            this->SyncInvoke(method, request);
#ifdef __DEBUG__
            LOG_INFO("Sync OnResponse " << protocolConfig->Service
                                        << "." << protocolConfig->Method << " [" << elapsedTimer.GetMs()
                                        << "ms]");
#endif
            return true;
        }

        if (method->IsLuaMethod()) //lua 异步
        {
            return true;
        }
        this->mCorComponent->Start([this, method, request, protocolConfig]()
        {
#ifdef __DEBUG__
            ElapsedTimer elapsedTimer;
#endif
            this->AsyncInvoke(method, request);
#ifdef __DEBUG__
            LOG_INFO("Async call " << protocolConfig->Service
                                   << "." << protocolConfig->Method << " [" << elapsedTimer.GetMs()
                                   << "ms]");
#endif
        });
        return true;
    }

    void ProtoRpcComponent::SyncInvoke(ServiceMethod *method, const com::Rpc_Request *request)
    {
        auto response = new com::Rpc_Response();
        method->SetSocketId(request->socketid());
        LocalObject<com::Rpc_Request> lock(request);
        XCode code = method->Invoke(*request, *response);
        if (request->rpcid() != 0)
        {
            response->set_code(code);
            response->set_rpcid(request->rpcid());
            response->set_userid(request->userid());
            this->mRpcClientComponent->SendByAddress(request->socketid(), response);
            return;
        }
        delete response;
    }

    bool ProtoRpcComponent::OnResponse(const com::Rpc_Response *response)
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
        rpcTask->OnResponse(response);
        return true;
    }

    unsigned int ProtoRpcComponent::AddRpcTask(std::shared_ptr<ProtoRpcTask> task)
    {
        int methodId = task->GetMethodId();
        long long rpcId = task->GetRpcId();
        this->mRpcTasks.emplace(rpcId, task);
        auto config = this->mPpcConfigComponent->GetProtocolConfig(methodId);
        if (config != nullptr && config->Timeout > 0)
        {
            return this->mTimerComponent->AsyncWait(
                    config->Timeout, &ProtoRpcComponent::OnTaskTimeout, this, rpcId);
        }
        return 0;
    }


	void ProtoRpcComponent::AsyncInvoke(ServiceMethod *method, const com::Rpc_Request *request)
    {
		auto response = new com::Rpc_Response();
		LocalObject<com::Rpc_Request> obj(request);
        XCode code = method->Invoke(*request, *response);
		if (request->rpcid() != 0)
		{
			response->set_code(code);
			response->set_rpcid(request->rpcid());
			response->set_userid(request->userid());
			this->mRpcClientComponent->SendByAddress(request->socketid(), response);
		}
    }

    long long ProtoRpcComponent::GetRpcTaskId()
    {
#ifdef __DEBUG__
        return mTick++;
#else
        long long nowTime = Helper::Time::GetSecTimeStamp();
        if(nowTime != this->mLastTime)
        {
            this->mTick = 0;
            this->mLastTime = nowTime;
        }
        return nowTime << 32 | (this->mTick++);
#endif
    }

    std::shared_ptr<ProtoRpcTask> ProtoRpcComponent::GetRpcTask(long long int rpcId) const
    {
        auto iter = this->mRpcTasks.find(rpcId);
        return iter != this->mRpcTasks.end() ? iter->second : nullptr;
    }

    void ProtoRpcComponent::OnTaskTimeout(long long int rpcId)
    {
        auto iter = this->mRpcTasks.find(rpcId);
        if (iter != this->mRpcTasks.end())
        {
            auto rpcTask = iter->second;
            this->mRpcTasks.erase(iter);
            rpcTask->OnResponse(nullptr);
        }
    }
}// namespace GameKeeper
