//
// Created by yjz on 2022/4/23.
//

#include"GateProxyComponent.h"
#include"Pool/MessagePool.h"
#include"Component/Gate/GateService.h"
#include"Script/LuaParameter.h"
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
		return this->mGateService->Call(userId, "CallClient", request);
	}

	XCode GateProxyComponent::Call(long long userId, const std::string& func, const Message& message)
	{
		c2s::Rpc::Call request;
		request.set_func(func);
		request.mutable_data()->PackFrom(message);
		return this->mGateService->Call(userId, "CallClient", request);
	}

	XCode GateProxyComponent::LuaCall(long long userId, const std::string func, std::shared_ptr<Message> message)
	{
		std::string address;
		if (!this->mGateService->GetAddressProxy().GetAddress(userId, address))
		{
			LOG_ERROR("not find user gate address : " << userId);
			return XCode::NotFindUser;
		}

		if(this->mGateService->SocketIsOpen(address))
		{
			c2s::Rpc::Call callInfo;
			callInfo.set_func(func);
			callInfo.mutable_data()->PackFrom(*message);
			return this->mGateService->Send(userId, "CallClient", callInfo);
		}

		TaskComponent* taskComponent = this->GetApp()->GetTaskComponent();
		taskComponent->Start([func, message, this, address, userId]()
		{
			c2s::Rpc::Call callInfo;
			callInfo.set_func(func);
			callInfo.mutable_data()->PackFrom(*message);
			this->mGateService->Send(userId, "CallClient", callInfo);
		});
		return XCode::Successful;
	}

	XCode GateProxyComponent::BroadCast(const std::string& func)
	{
		c2s::Rpc::Call request;
		request.set_func(func);
		//TODO

		return XCode::Successful;
	}

	XCode GateProxyComponent::BroadCast(const std::string& func, const Message& message)
	{
		s2s::GateBroadCast::Request request;
		request.set_func(func);
		std::list<std::string> allAddress;
		request.mutable_data()->PackFrom(message);
		return this->mGateService->Send("BroadCast", request);
	}

	XCode GateProxyComponent::LuaBroadCast(const std::string func, std::shared_ptr<Message> message)
	{
		TaskComponent* taskComponent = this->GetApp()->GetTaskComponent();
		taskComponent->Start([this, func, message]()
		{
			s2s::GateBroadCast::Request request;
			request.set_func(func);
			request.mutable_data()->PackFrom(*message);
			this->mGateService->Send("BroadCast", request);
		});
		return XCode::Successful;
	}

	void GateProxyComponent::OnLuaRegister(Lua::ClassProxyHelper & luaRegister)
	{
		luaRegister.BeginRegister<GateProxyComponent>();
		luaRegister.PushMemberFunction("Call", &GateProxyComponent::LuaCall);
		luaRegister.PushMemberFunction("BroadCast", &GateProxyComponent::LuaBroadCast);
	}
}