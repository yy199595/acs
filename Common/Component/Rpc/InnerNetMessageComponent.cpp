#include"InnerNetMessageComponent.h"
#include"Component/Coroutine/TaskComponent.h"
#include"Util/StringHelper.h"
#include"App/App.h"
#include"Method/LuaServiceMethod.h"
#include"Global/ServiceConfig.h"
#include"Component/Rpc/InnerNetComponent.h"
#include"Other/ElapsedTimer.h"
#include"Component/RpcService/LocalService.h"
namespace Sentry
{
	void InnerNetMessageComponent::Awake()
	{
		this->mTaskComponent = nullptr;
		this->mTimerComponent = nullptr;
		this->mRpcClientComponent = nullptr;
	}

	bool InnerNetMessageComponent::LateAwake()
	{
		this->mTaskComponent = App::Get()->GetTaskComponent();
		this->mTimerComponent = this->GetComponent<TimerComponent>();
		LOG_CHECK_RET_FALSE(this->mTaskComponent = this->GetComponent<TaskComponent>());
		LOG_CHECK_RET_FALSE(this->mRpcClientComponent = this->GetComponent<InnerNetComponent>());
		return true;
	}

    bool InnerNetMessageComponent::OnProtoRequest(const std::string &address, const char *data, int len)
    {
        std::shared_ptr<com::rpc::request> request(new com::rpc::request());
        if(!request->ParseFromArray(data, len))
        {
            return false;
        }
        request->set_address(address);
        request->set_type(com::rpc_msg_type_proto);
        return this->OnRequest(request) == XCode::Successful;
    }

    bool InnerNetMessageComponent::OnProtoResponse(const std::string &address, const char *data, int len)
    {
        std::shared_ptr<com::rpc::response> response(new com::rpc::response());
        if(!response->ParseFromArray(data, len))
        {
            return false;
        }
        long long taskId = response->rpc_id();
        this->OnResponse(taskId, response);
        return true;
    }

	XCode InnerNetMessageComponent::OnRequest(std::shared_ptr<com::rpc::request> request)
	{
		if(!RpcServiceConfig::ParseFunName(request->func(), this->mService, this->mMethod))
		{
			return XCode::NotFoundRpcConfig;
		}
		Service * logicService = this->GetApp()->GetService(this->mService);
		if (logicService == nullptr || !logicService->IsStartService())
		{
			LOG_ERROR("call service not exist : [" << this->mService << "]");
			return XCode::CallServiceNotFound;
		}
		const RpcServiceConfig & rpcServiceConfig = logicService->GetServiceConfig();
		const RpcInterfaceConfig* rpcInterfaceConfig = rpcServiceConfig.GetConfig(this->mMethod);
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
			if(request->rpc_id() != 0)
			{
                response->set_code((int)code);
                response->set_rpc_id(request->rpc_id());
                this->mRpcClientComponent->Send(request->address(), response);
			}
			return XCode::Successful;
		}

		this->mTaskComponent->Start([request, this, logicService, rpcInterfaceConfig, response]()
		{
			ElapsedTimer elapsedTimer;
			const std::string& func = rpcInterfaceConfig->Method;
			XCode code = logicService->Invoke(func, request, response);
			if(request->rpc_id() != 0)
            {
                response->set_code((int) code);
                response->set_rpc_id(request->rpc_id());
                this->mRpcClientComponent->Send(request->address(), response);
            }
		});
		return XCode::Successful;
	}
}// namespace Sentry
