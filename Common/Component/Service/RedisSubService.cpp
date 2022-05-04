//
// Created by yjz on 2022/1/22.
//
#include"RedisSubService.h"
#include"Component/Redis/MainRedisComponent.h"
namespace Sentry
{
	bool RedisSubService::LateAwake()
	{
		RemoteServiceComponent::LateAwake();
		this->mRedisComponent = this->GetComponent<MainRedisComponent>();
		this->GetConfig().GetListenerAddress("rpc", this->mLocalAddress);
		return true;
	}
	void RedisSubService::GetSubMethods(std::vector<std::string>& methods)
	{
		if(this->mServiceRegister != nullptr)
		{
			this->mServiceRegister->GetMethods(methods);
		}
	}

	bool RedisSubService::LoadService()
	{
		this->mServiceRegister = std::make_shared<SubServiceRegister>(this);
		return this->OnInitService(*this->mServiceRegister);
	}

	void RedisSubService::OnAddAddress(const std::string& address)
	{

	}

	XCode RedisSubService::Send(const std::string& address, std::shared_ptr<Rpc_Request> message)
	{
		std::string request = "+";
		message->set_address(this->mLocalAddress);
		if(!message->AppendToString(&request))
		{
			return XCode::SerializationFailure;
		}
		if(this->mRedisComponent->Publish(address, request) > 0)
		{
			return XCode::Successful;
		}
		return XCode::SendMessageFail;
	}

	XCode RedisSubService::Send(const std::string& address, std::shared_ptr<Rpc_Response> message)
	{
		std::string response = "-";
		if(!message->AppendToString(&response))
		{
			return XCode::SerializationFailure;
		}
		if(this->mRedisComponent->Publish(address, response) > 0)
		{
			return XCode::Successful;
		}
		return XCode::SendMessageFail;
	}

	XCode RedisSubService::Invoke(const string& func, std::shared_ptr<Rpc_Request> request,
	    std::shared_ptr<Rpc_Response> response)
	{
		if(!this->IsStartService())
		{
			return XCode::CallServiceNotFound;
		}
		std::shared_ptr<SubMethod> subMethod = this->mServiceRegister->GetMethod(func);
		if(subMethod == nullptr)
		{
			return XCode::CallFunctionNotExist;
		}
		try
		{
			return subMethod->OnPublish(*request, *response);
		}
		catch(std::logic_error & err)
		{
			response->set_error_str(err.what());
			return XCode::ThrowError;
		}
	}
	XCode RedisSubService::SendRequest(const string& address, std::shared_ptr<com::Rpc::Request> request)
	{
		return this->Send(address, request);
	}
}
