#include"RpcHandlerComponent.h"
#include<Component/RpcService/LocalServiceComponent.h>
#include"Component/Coroutine/TaskComponent.h"
#include"Util/StringHelper.h"
#include"App/App.h"
#include"Method/LuaServiceMethod.h"
#include"Global/ServiceConfig.h"
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
		const ServiceConfig & rpcConfig = this->GetApp()->GetServiceConfig();
		const RpcInterfaceConfig* rpcInterfaceConfig = rpcConfig.GetInterfaceConfig(methodId);
		if (rpcInterfaceConfig == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}

		const std::string& service = rpcInterfaceConfig->Service;
		LocalServiceComponent * logicService = this->GetComponent<LocalServiceComponent>(service);
		if (logicService == nullptr)
		{
			LOG_ERROR("call service not exist : [" << service << "]");
			return XCode::CallServiceNotFound;
		}
		std::shared_ptr<com::Rpc::Response> response(new com::Rpc::Response());
		response->set_rpc_id(request->rpc_id());
		response->set_user_id(request->user_id());

		if (!rpcInterfaceConfig->IsAsync)
		{
			const std::string& func = rpcInterfaceConfig->Method;
			response->set_code((int)logicService->Invoke(func, request, response));
			if(rpcInterfaceConfig->CallWay == "Sub")
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

		this->mCorComponent->Start([request, this, logicService, rpcInterfaceConfig, response]()
		{
			const std::string& func = rpcInterfaceConfig->Method;
			response->set_code((int)logicService->Invoke(func, request, response));
			if(rpcInterfaceConfig->CallWay == "Sub")
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
		std::shared_ptr<IRpcTask> rpcTask = iter->second;
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
			this->mTimerComponent->DelayCall(task->GetTimeout(),
				&RpcHandlerComponent::OnTaskTimeout, this, rpcId);
		}
	}

	void RpcHandlerComponent::OnTaskTimeout(long long int rpcId)
	{
		auto iter = this->mRpcTasks.find(rpcId);
		if (iter != this->mRpcTasks.end())
		{
			auto rpcTask = iter->second;
			this->mRpcTasks.erase(iter);
			rpcTask->OnResponse(nullptr);
		}
	}
}// namespace Sentry
