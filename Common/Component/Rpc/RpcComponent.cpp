#include"RpcComponent.h"
#include<Component/Service/RpcService.h>
#include"Component/Coroutine/TaskComponent.h"
#include"Util/StringHelper.h"
#include"App/App.h"
#include"Pool/MessagePool.h"
#include"Method/LuaServiceMethod.h"
#include"Component/Rpc/RpcConfigComponent.h"
#include"Component/Rpc/RpcClientComponent.h"
#include"Other/ElapsedTimer.h"
#include"Json/JsonWriter.h"
#include"Async/RpcTask/RpcTaskSource.h"
namespace Sentry
{
	bool RpcComponent::Awake()
	{
		this->mCorComponent = nullptr;
		this->mTimerComponent = nullptr;
		this->mPpcConfigComponent = nullptr;
		this->mRpcClientComponent = nullptr;
		return true;
	}

	bool RpcComponent::LateAwake()
	{
		this->mCorComponent = App::Get()->GetTaskComponent();
		this->mTimerComponent = this->GetComponent<TimerComponent>();
		LOG_CHECK_RET_FALSE(this->mCorComponent = this->GetComponent<TaskComponent>());
		LOG_CHECK_RET_FALSE(this->mPpcConfigComponent = this->GetComponent<RpcConfigComponent>());
		LOG_CHECK_RET_FALSE(this->mRpcClientComponent = this->GetComponent<RpcClientComponent>());
		return true;
	}

	XCode RpcComponent::OnRequest(std::shared_ptr<com::Rpc_Request> request)
	{
		unsigned short methodId = request->method_id();
		const ProtoConfig* protocolConfig = this->mPpcConfigComponent->GetProtocolConfig(methodId);
		if (protocolConfig == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}

		const std::string& service = protocolConfig->Service;
		auto logicService = this->GetComponent<RpcService>(service);
		if (logicService == nullptr)
		{
			LOG_ERROR("call service not exist : [", service, "]");
			return XCode::CallServiceNotFound;
		}
#ifdef __DEBUG__
		std::string json = "";
		LOG_DEBUG("==============[request]==============");
		LOG_DEBUG("func = {0}.{1}", protocolConfig->Service, protocolConfig->Method);
		if (request->has_data() && Helper::Proto::GetJson(request->data(), json))
		{
			LOG_DEBUG("json = {}", json);
		}
		LOG_DEBUG("=====================================");
#endif

		if (!protocolConfig->IsAsync)
		{
			long long socketId = request->socket_id();
			const std::string& method = protocolConfig->Method;
			auto response = logicService->Invoke(method, request);
			this->mRpcClientComponent->Send(socketId, response);
			return XCode::Successful;
		}
		this->mCorComponent->Start([request, this, logicService, protocolConfig]()
		{
			long long socketId = request->socket_id();
			const std::string& method = protocolConfig->Method;
			auto response = logicService->Invoke(method, request);
			this->mRpcClientComponent->Send(socketId, response);
		});
		return XCode::Successful;
	}

	XCode RpcComponent::OnResponse(std::shared_ptr<com::Rpc_Response> response)
	{
		long long rpcId = response->rpc_id();
		auto iter = this->mRpcTasks.find(rpcId);
		if (iter == this->mRpcTasks.end())
		{
			LOG_WARN("not find rpc task : ", rpcId)
			return XCode::Failure;
		}
		auto rpcTask = iter->second;
#ifdef __DEBUG__
		int methodId = 0;
		long long time = 0;
		if (this->GetRpcInfo(rpcId, methodId, time))
		{
			std::string json = "";
			RpcConfigComponent* configComponent = this->GetComponent<RpcConfigComponent>();
			const ProtoConfig* protoConfig = configComponent->GetProtocolConfig(methodId);

			LOG_DEBUG("*************[response]*************");
			LOG_DEBUG("func = {0}.{1}", protoConfig->Service, protoConfig->Method);
			LOG_DEBUG("time = {0}ms", time);
			if (response->has_data() && Helper::Proto::GetJson(response->data(), json))
			{
				LOG_DEBUG("json = {}", json);
			}
			LOG_DEBUG("************************************");
		}
#endif
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

	bool RpcComponent::GetRpcInfo(long long int rpcId, int& methodId, long long int& time)
	{
		auto iter = this->mRpcInfoMap.find(rpcId);
		if (iter == this->mRpcInfoMap.end())
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
			LOG_ERROR("{0} time out", rpcTask->GetRpcId());
		}
#ifdef __DEBUG__
		int methodId = 0;
		long long costTime = 0;
		if (this->GetRpcInfo(rpcId, methodId, costTime))
		{
			auto config = this->mPpcConfigComponent->GetProtocolConfig(methodId);
			if (config != nullptr)
			{
				LOG_ERROR("call ", config->Service, '.', config->Method, " time out");
			}
		}
#endif
	}
}// namespace Sentry
