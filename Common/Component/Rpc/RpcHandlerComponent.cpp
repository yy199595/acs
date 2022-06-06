#include"RpcHandlerComponent.h"
#include"Component/Coroutine/TaskComponent.h"
#include"Util/StringHelper.h"
#include"App/App.h"
#include"Method/LuaServiceMethod.h"
#include"Global/ServiceConfig.h"
#include"Component/Rpc/RpcClientComponent.h"
#include"Other/ElapsedTimer.h"
#include"Json/JsonWriter.h"
#include"Async/RpcTask/RpcTaskSource.h"
#include"Component/RpcService/LocalServiceComponent.h"
namespace Sentry
{
	void RpcHandlerComponent::Awake()
	{
		this->mTaskComponent = nullptr;
		this->mTimerComponent = nullptr;
		this->mRpcClientComponent = nullptr;
	}

	bool RpcHandlerComponent::LateAwake()
	{
		this->mTaskComponent = App::Get()->GetTaskComponent();
		this->mTimerComponent = this->GetComponent<TimerComponent>();
		LOG_CHECK_RET_FALSE(this->mTaskComponent = this->GetComponent<TaskComponent>());
		LOG_CHECK_RET_FALSE(this->mRpcClientComponent = this->GetComponent<RpcClientComponent>());
		return true;
	}

	XCode RpcHandlerComponent::OnRequest(std::shared_ptr<com::Rpc_Request> request)
	{
		if(!RpcServiceConfig::ParseFunName(request->func(), this->mTempService, this->mTempMethod))
		{
			return XCode::NotFoundRpcConfig;
		}
		ServiceComponent * logicService = this->GetApp()->GetService(this->mTempService);
		if (logicService == nullptr || !logicService->IsStartService())
		{
			LOG_ERROR("call service not exist : [" << this->mTempService << "]");
			return XCode::CallServiceNotFound;
		}
		const RpcServiceConfig & rpcServiceConfig = logicService->GetServiceConfig();
		const RpcInterfaceConfig* rpcInterfaceConfig = rpcServiceConfig.GetConfig(this->mTempMethod);
		if (rpcInterfaceConfig == nullptr)
		{
			return XCode::NotFoundRpcConfig;
		}
		if(!rpcInterfaceConfig->Request.empty())
		{
			if(!request->has_data())
			{
				LOG_ERROR("call " << rpcInterfaceConfig->FullName << " not proto data"
					<< " need proto type " << rpcInterfaceConfig->Request);
				return XCode::CallArgsError;
			}
			std::string fullName;
			if(!Any::ParseAnyTypeUrl(request->data().type_url(), &fullName)
				|| rpcInterfaceConfig->Request != fullName)
			{
				LOG_ERROR("call " << rpcInterfaceConfig->FullName << " need "
					<< rpcInterfaceConfig->FullName << " but use " << fullName);
				return XCode::CallArgsError;
			}
		}

		const std::string& service = rpcInterfaceConfig->Service;

		std::shared_ptr<com::Rpc::Response> response = std::make_shared<com::Rpc::Response>();
		if (!rpcInterfaceConfig->IsAsync)
		{
			ElapsedTimer elapsedTimer;
			const std::string& func = rpcInterfaceConfig->Method;
			XCode code = logicService->Invoke(func, request, response);
			if(request->rpc_id() == 0)
			{
				return XCode::Successful;
			}
			response->set_code((int)code);
			response->set_rpc_id(request->rpc_id());
			response->set_user_id(request->user_id());
			this->mRpcClientComponent->Send(request->address(), response);
			LOG_INFO("call " << rpcInterfaceConfig->FullName << " use time [" << elapsedTimer.GetMs() << "ms]");
			return XCode::Successful;
		}

		this->mTaskComponent->Start([request, this, logicService, rpcInterfaceConfig, response]()
		{
			ElapsedTimer elapsedTimer;
			const std::string& func = rpcInterfaceConfig->Method;
			XCode code = logicService->Invoke(func, request, response);
			if(request->rpc_id() == 0)
			{
				return XCode::Successful;
			}
			response->set_code((int)code);
			response->set_rpc_id(request->rpc_id());
			response->set_user_id(request->user_id());
			this->mRpcClientComponent->Send(request->address(), response);
			LOG_INFO("async call " << rpcInterfaceConfig->FullName << " use time [" << elapsedTimer.GetMs() << "ms]");
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
