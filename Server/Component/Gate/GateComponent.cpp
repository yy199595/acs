//
// Created by mac on 2021/11/28.
//

#include"GateComponent.h"
#include"App/App.h"
#include"NetWork/GateRpcClientContext.h"
#include"Global/RpcConfig.h"
#include"Task/RpcProxyTask.h"
#include"Component/Rpc/RpcComponent.h"
#include"GateClientComponent.h"
#include"Component/Rpc/RpcClientComponent.h"
#include"Component/RpcService/LocalServiceComponent.h"
#ifdef __DEBUG__
#include"Pool/MessagePool.h"
#include"google/protobuf/util/json_util.h"
#endif
#include"Component/Service/UserSubService.h"
namespace Sentry
{

	bool GateComponent::LateAwake()
	{
		this->mTaskComponent = this->GetApp()->GetTaskComponent();
		this->mTimerComponent = this->GetApp()->GetTimerComponent();
		LOG_CHECK_RET_FALSE(this->mGateClientComponent = this->GetComponent<GateClientComponent>());
		return true;
	}

	void GateComponent::OnConnect(const std::string & address)
	{
		unsigned int timerId = this->mTimerComponent->DelayCall(5000,
				&GateComponent::OnSocketTimeout, this, address);
		this->mSocketTimers.emplace(address, timerId);
	}

	void GateComponent::OnSocketTimeout(const std::string & address)
	{
		auto iter = this->mSocketTimers.find(address);
		if(iter != this->mSocketTimers.end())
		{
			this->mSocketTimers.erase(iter);
			this->mGateClientComponent->StartClose(address);
		}
	}

	XCode GateComponent::OnRequest(std::shared_ptr<c2s::Rpc_Request> request)
	{
		long long userId = 0;
		const std::string &address = request->address();
		if(!this->mGateClientComponent->GetUserId(address, userId))
		{
			return this->Auth(request);
		}
		const RpcConfig & rpcConfig = this->GetApp()->GetRpcConfig();
		const ProtoConfig * config = rpcConfig.GetProtocolConfig(request->method_name());
		if (config == nullptr || config->Type != "Client")
		{
			LOG_ERROR("call function " << request->method_name() << " not find");
			return XCode::NotFoundRpcConfig;
		}
		if(!config->Request.empty())
		{
			if(!request->has_data())
			{
				return XCode::CallArgsError;
			}
			std::string fullName;
			if(!Any::ParseAnyTypeUrl(request->data().type_url(), &fullName)
				|| fullName != config->Request)
			{
				return XCode::CallArgsError;
			}
		}
		else if(request->has_data())
		{
			return XCode::CallArgsError;
		}

//#ifdef __DEBUG__
//		std::string json;
//		LOG_INFO("========== client request ==========");
//		LOG_INFO("func = " << request->method_name());
//		LOG_INFO("address = " << request->address());
//		LOG_INFO("userid = " << userId);
//		if(request->has_data() && Helper::Proto::GetJson(request->data(), json))
//		{
//			LOG_INFO("json = " << json);
//		}
//#endif
		if(this->GetComponent<LocalServiceComponent>(config->Service) == nullptr)
		{
			return XCode::CallServiceNotFound;
		}
		this->mTaskComponent->Start(&GateComponent::OnUserRequest, this, userId, request);
		return XCode::Successful;
	}

	bool GateComponent::AddUserToken(const std::string& token, long long userId)
	{
		auto iter = this->mUserTokens.find(token);
		if(iter == this->mUserTokens.end())
		{
			this->mUserTokens.emplace(token, userId);
			return true;
		}
		return false;
	}

	XCode GateComponent::Auth(const std::shared_ptr<c2s::Rpc::Request> request)
	{
		c2s::GateLogin::Request loginRequest;
		if (!request->has_data() || !request->data().Is<c2s::GateLogin::Request>()
			|| !request->mutable_data()->UnpackTo(&loginRequest))
		{
			return XCode::CallArgsError;
		}
		const std::string& token = loginRequest.token();
		const std::string & address = request->address();

		auto iter = this->mUserTokens.find(token);
		if (iter == this->mUserTokens.end())
		{
			return XCode::Failure;
		}
		long long userId = iter->second;
		this->mUserTokens.erase(iter);

		auto iter1 = this->mSocketTimers.find(address);
		if(iter1 != this->mSocketTimers.end())
		{
			unsigned int id = iter1->second;
			this->mSocketTimers.erase(iter1);
			this->mTimerComponent->CancelTimer(id);
		}

		LOG_INFO(userId << " login gate successful");
		this->mGateClientComponent->AddNewUser(request->address(), userId);
		std::shared_ptr<c2s::Rpc::Response> clientResponse(new c2s::Rpc::Response());

		clientResponse->set_rpc_id(request->rpc_id());
		clientResponse->set_code((int)XCode::Successful);
		this->mGateClientComponent->SendToClient(address, clientResponse);
		return XCode::Successful;

	}

	XCode GateComponent::OnResponse(const std::string & address, std::shared_ptr<c2s::Rpc_Response> response)
	{
#ifdef __DEBUG__
		LOG_WARN("**********[client response]**********");
		if (response->has_data())
		{
			std::string json;
			if ( Helper::Proto::GetJson(response->data(), json))
			{
				LOG_WARN("json = " << json);
			}
		}
		LOG_WARN("*****************************************");
#endif
		if (this->mGateClientComponent->SendToClient(address, response))
		{
			return XCode::NetWorkError;
		}
		return XCode::Successful;
	}

	void GateComponent::OnUserRequest(long long userId, std::shared_ptr<c2s::Rpc::Request> request)
	{
		const RpcConfig& rpcConfig = this->GetApp()->GetRpcConfig();
		const ProtoConfig* config = rpcConfig.GetProtocolConfig(request->method_name());
		if (config == nullptr)
		{
			return;
		}
		std::string address;
		LocalServiceComponent* localServerRpc = this->GetComponent<LocalServiceComponent>(config->Service);
		if (!localServerRpc->GetEntityAddress(userId, address))
		{
			localServerRpc->AllotAddress(address);
			if(!localServerRpc->AddEntity(userId, address, true))
			{
				LOG_ERROR(userId << " publish failure");
				return;
			}
		}
		std::shared_ptr<com::Rpc::Request> requestData(new com::Rpc::Request());
		requestData->set_user_id(userId);
		requestData->set_method_id(config->MethodId);
		requestData->mutable_data()->PackFrom(request->data());
		std::shared_ptr<c2s::Rpc::Response> clientResponse(new c2s::Rpc::Response());
		std::shared_ptr<com::Rpc::Response> responseData = localServerRpc->StartCall(address, requestData);
		if(responseData != nullptr)
		{
			clientResponse->set_rpc_id(request->rpc_id());
			clientResponse->set_code(responseData->code());
			clientResponse->set_error_str(responseData->error_str());
			clientResponse->mutable_data()->PackFrom(responseData->data());
			this->mGateClientComponent->SendToClient(request->address(), clientResponse);
		}
	}
}