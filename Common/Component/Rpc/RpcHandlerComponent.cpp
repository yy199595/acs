#include"RpcHandlerComponent.h"
#include<Component/RpcService/LocalServiceComponent.h>
#include"Component/Coroutine/TaskComponent.h"
#include"Util/StringHelper.h"
#include"App/App.h"
#include"Method/LuaServiceMethod.h"
#include"Global/RpcConfig.h"
#include"Component/Rpc/RpcClientComponent.h"
#include"Other/ElapsedTimer.h"
#include"Json/JsonWriter.h"
#include"Async/RpcTask/RpcTaskSource.h"
#include"Component/Redis/MainRedisComponent.h"
namespace Sentry
{
	void RpcHandlerComponent::Awake()
	{
		this->mCorComponent = nullptr;
		this->mTimerComponent = nullptr;
		this->mRpcClientComponent = nullptr;
	}

	bool RpcHandlerComponent::LateAwake()
	{
		this->mCorComponent = App::Get()->GetTaskComponent();
		this->mTimerComponent = this->GetComponent<TimerComponent>();
		this->mRedisComponent = this->GetComponent<MainRedisComponent>();
		LOG_CHECK_RET_FALSE(this->mCorComponent = this->GetComponent<TaskComponent>());
		LOG_CHECK_RET_FALSE(this->mRpcClientComponent = this->GetComponent<RpcClientComponent>());
		return true;
	}

	XCode RpcHandlerComponent::OnRequest(std::shared_ptr<com::Rpc_Request> request)
	{
		unsigned short methodId = request->method_id();
		const RpcConfig & rpcConfig = this->GetApp()->GetRpcConfig();
		const ProtoConfig* protocolConfig = rpcConfig.GetProtocolConfig(methodId);
		if (protocolConfig == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}

		const std::string& service = protocolConfig->Service;
		typedef ICallService<com::Rpc::Request, com::Rpc::Response> CallService;
		CallService * logicService = this->GetComponent<CallService>(service);
		if (logicService == nullptr)
		{
			LOG_ERROR("call service not exist : [" << service << "]");
			return XCode::CallServiceNotFound;
		}
		std::shared_ptr<com::Rpc::Response> response(new com::Rpc::Response());
		response->set_rpc_id(request->rpc_id());
		response->set_user_id(request->user_id());

		if (!protocolConfig->IsAsync)
		{
			const std::string& func = protocolConfig->Method;
			response->set_code((int)logicService->Invoke(func, request, response));
			if(protocolConfig->CallWay == "Sub")
			{
				std::string message = "-";
				if(response->AppendToString(&message))
				{
					const std::string & channel = request->address();
					long long num = this->mRedisComponent->Publish(channel, message);
					return num == 1 ? XCode::Successful : XCode::SendMessageFail;
				}
				return XCode::SerializationFailure;
			}
			this->mRpcClientComponent->Send(request->address(), response);
			return XCode::Successful;
		}

		this->mCorComponent->Start([request, this, logicService, protocolConfig, response]()
		{
			const std::string& func = protocolConfig->Method;
			response->set_code((int)logicService->Invoke(func, request, response));
			if(protocolConfig->CallWay == "Sub")
			{
				std::string message = "-";
				if(response->AppendToString(&message))
				{
					const std::string & channel = request->address();
					long long num = this->mRedisComponent->Publish(channel, message);
					return num == 1 ? XCode::Successful : XCode::SendMessageFail;
				}
				return XCode::SerializationFailure;
			}
			this->mRpcClientComponent->Send(request->address(), response);
			return XCode::Successful;
		});
		return XCode::Successful;
	}

	XCode RpcHandlerComponent::OnResponse(std::shared_ptr<com::Rpc_Response> response)
	{
		long long rpcId = response->rpc_id();
		auto iter = this->mRpcTasks.find(rpcId);
		if (iter == this->mRpcTasks.end())
		{
			LOG_WARN("not find rpc task : "<< rpcId)
			return XCode::Failure;
		}
		auto rpcTask = iter->second;
//#ifdef __DEBUG__
//		int methodId = 0;
//		long long time = 0;
//		if (this->GetRpcInfo(rpcId, methodId, time))
//		{
//			std::string json = "";
//			const RpcConfig & rpcConfig = this->GetApp()->GetRpcConfig();
//			const ProtoConfig* protoConfig = rpcConfig.GetProtocolConfig(methodId);
//
//			LOG_DEBUG("*************[response]*************");
//			LOG_DEBUG("func = " << protoConfig->Service << '.' << protoConfig->Method);
//			LOG_DEBUG("time = " << time << " ms");
//			if (response->has_data() && Helper::Proto::GetJson(response->data(), json))
//			{
//				LOG_DEBUG("json = " << json);
//			}
//			LOG_DEBUG("************************************");
//		}
//#endif
		this->mRpcTasks.erase(iter);
		rpcTask->OnResponse(response);
		return XCode::Successful;
	}

	void RpcHandlerComponent::AddRpcTask(std::shared_ptr<IRpcTask> task)
	{
		long long rpcId = task->GetRpcId();
		this->mRpcTasks.emplace(rpcId, task);
		if (task->GetTimeout() > 0)
		{
			this->mTimerComponent->DelayCall(
					task->GetTimeout(), &RpcHandlerComponent::OnTaskTimeout, this, rpcId);
		}
	}
#ifdef __DEBUG__
	void RpcHandlerComponent::AddRpcInfo(long long rpcId, int methodId)
	{
		RpcTaskInfo taskInfo;
		taskInfo.MethodId = methodId;
		taskInfo.Time = Helper::Time::GetNowMilTime();
		this->mRpcInfoMap.emplace(rpcId, taskInfo);
	}

	bool RpcHandlerComponent::GetRpcInfo(long long int rpcId, int& methodId, long long int& time)
	{
		auto iter = this->mRpcInfoMap.find(rpcId);
		if (iter == this->mRpcInfoMap.end())
		{
			return false;
		}
		methodId = iter->second.MethodId;
		time = Helper::Time::GetNowMilTime() - iter->second.Time;
		this->mRpcInfoMap.erase(iter);
		return true;
	}
#endif

	void RpcHandlerComponent::OnTaskTimeout(long long int rpcId)
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
		if (this->GetRpcInfo(rpcId, methodId, costTime))
		{
			const RpcConfig & rpcConfig = this->GetApp()->GetRpcConfig();
			const ProtoConfig* config = rpcConfig.GetProtocolConfig(methodId);
			if (config != nullptr)
			{
				LOG_ERROR("call " << config->Service << '.' << config->Method << " time out");
			}
		}
#endif
	}
}// namespace Sentry
