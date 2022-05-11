//
// Created by yjz on 2022/4/23.
//

#include"GateProxyComponent.h"
#include"Component/Gate/GateService.h"
namespace Sentry
{
	bool GateProxyComponent::LateAwake()
	{
		this->mGateService = this->GetComponent<GateService>();
		return true;
	}


	XCode GateProxyComponent::Call(long long userId, const std::string& func)
	{
		c2s::Rpc::Call request;
		request.set_func(func);
		return this->mGateService->Call("CallClient", userId, request);
	}

	XCode GateProxyComponent::Call(long long userId, const std::string& func, const Message& message)
	{
		c2s::Rpc::Call request;
		request.set_func(func);
		request.mutable_data()->PackFrom(message);
		return this->mGateService->Call("CallClient", userId, request);
	}

	XCode GateProxyComponent::BroadCast(const std::string& func)
	{
		c2s::Rpc::Call request;
		request.set_func(func);
		std::list<std::string> addAddress;
		this->mGateService->GetAllAddress(addAddress);
		for(const std::string & address : addAddress)
		{
			this->mGateService->Call(address,"CallClient", request);
		}
		return XCode::Successful;
	}

	XCode GateProxyComponent::BroadCast(const std::string& func, const Message& message)
	{
		c2s::Rpc::Call request;
		request.set_func(func);
		std::list<std::string> allAddress;
		request.mutable_data()->PackFrom(message);
		this->mGateService->GetAllAddress(allAddress);
		for(const std::string & address : allAddress)
		{
			return this->mGateService->Call(address,"CallClient", request);
		}
		return XCode::Successful;
	}
}