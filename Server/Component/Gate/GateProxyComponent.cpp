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

	XCode GateProxyComponent::LuaCall(long long userId, const std::string func, const std::string pb, const std::string& json)
	{
		std::string address;
		if(this->mGateService->GetEntityAddress(userId, address))
		{
			return XCode::NotFindUser;
		}
		std::shared_ptr<Message> message = Helper::Proto::NewByJson(pb, json);
		if(message == nullptr)
		{
			return XCode::JsonCastProtoFailure;
		}
		std::shared_ptr<c2s::Rpc::Call> request(new c2s::Rpc::Call());
		TaskComponent * taskComponent = this->GetApp()->GetTaskComponent();

		request->set_func(func);
		request->mutable_data()->PackFrom(*message);
		taskComponent->Start([userId, request, this]()
		{
			this->mGateService->Call(userId, "CallClient", request);
		});
		return XCode::Successful;
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
		s2s::GateBroadCast::Request request;
		request.set_func(func);
		std::list<std::string> allAddress;
		request.mutable_data()->PackFrom(message);
		this->mGateService->GetAllAddress(allAddress);
		for(const std::string & address : allAddress)
		{
			this->mGateService->Call(address,"BroadCast", request);
		}
		return XCode::Successful;
	}

	XCode GateProxyComponent::LuaBroadCast(const std::string func, const std::string pb, const std::string& json)
	{
		std::shared_ptr<Message> message = Helper::Proto::NewByJson(pb, json);
		if(message == nullptr)
		{
			return XCode::JsonCastProtoFailure;
		}

		TaskComponent * taskComponent = this->GetApp()->GetTaskComponent();
		taskComponent->Start([this, func, message]()
		{
			std::list<std::string> allAddress;
			if(this->mGateService->GetAllAddress(allAddress))
			{
				s2s::GateBroadCast::Request request;
				request.set_func(func);
				request.mutable_data()->PackFrom(*message);
				for (const std::string& address: allAddress)
				{
					this->mGateService->Call(address, "BroadCast", request);
				}
			}
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