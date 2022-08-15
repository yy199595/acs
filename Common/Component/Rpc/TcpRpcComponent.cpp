#include"TcpRpcComponent.h"
#include"Component/Coroutine/TaskComponent.h"
#include"Util/StringHelper.h"
#include"App/App.h"
#include"Method/LuaServiceMethod.h"
#include"Global/ServiceConfig.h"
#include"Component/Rpc/RpcServerComponent.h"
#include"Other/ElapsedTimer.h"
#include"Component/RpcService/LocalService.h"
namespace Sentry
{
	void TcpRpcComponent::Awake()
	{
		this->mTaskComponent = nullptr;
		this->mTimerComponent = nullptr;
		this->mRpcClientComponent = nullptr;
	}

	bool TcpRpcComponent::LateAwake()
	{
		this->mTaskComponent = App::Get()->GetTaskComponent();
		this->mTimerComponent = this->GetComponent<TimerComponent>();
		LOG_CHECK_RET_FALSE(this->mTaskComponent = this->GetComponent<TaskComponent>());
		LOG_CHECK_RET_FALSE(this->mRpcClientComponent = this->GetComponent<RpcServerComponent>());
		return true;
	}

	XCode TcpRpcComponent::OnRequest(std::shared_ptr<com::rpc::request> request)
	{
		if(!RpcServiceConfig::ParseFunName(request->func(), this->mTempService, this->mTempMethod))
		{
			return XCode::NotFoundRpcConfig;
		}
		Service * logicService = this->GetApp()->GetService(this->mTempService);
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
			this->mFullName.clear();
			if(!Any::ParseAnyTypeUrl(request->data().type_url(), &this->mFullName)
				|| rpcInterfaceConfig->Request != this->mFullName)
			{
				LOG_ERROR("call " << rpcInterfaceConfig->FullName << " need "
					<< rpcInterfaceConfig->FullName << " but use " << this->mFullName);
				return XCode::CallArgsError;
			}
		}

		const std::string& service = rpcInterfaceConfig->Service;
		std::shared_ptr<com::rpc::response> response = std::make_shared<com::rpc::response>();
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
			this->mRpcClientComponent->Send(request->address(), response);
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
			this->mRpcClientComponent->Send(request->address(), response);
			//LOG_INFO("async call " << rpcInterfaceConfig->FullName << " use time [" << elapsedTimer.GetMs() << "ms]");
			return XCode::Successful;
		});
		return XCode::Successful;
	}
}// namespace Sentry
