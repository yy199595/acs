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

	XCode GateProxyComponent::LuaCall(long long userId, const std::string func, const std::string proto, const std::string& content)
	{
		std::string address;
		if(this->mGateService->GetUserAddress(userId, address))
		{
			return XCode::NotFindUser;
		}
		std::shared_ptr<c2s::Rpc::Call> request(new c2s::Rpc::Call());

		request->set_func(func);
		if(proto == "json")
		{
			com::Type::Json message;
			message.set_json(content);
			request->mutable_data()->PackFrom(message);
		}
		else
		{
			std::shared_ptr<Message> message = Helper::Proto::NewByJson(proto, content);
			if(message == nullptr)
			{
				return XCode::JsonCastProtoFailure;
			}
			request->mutable_data()->PackFrom(*message);
		}
		TaskComponent * taskComponent = this->GetApp()->GetTaskComponent();
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

	XCode GateProxyComponent::LuaBroadCast(const std::string func, const std::string pb, const std::string& json)
	{
		std::shared_ptr<Message> message = Helper::Proto::NewByJson(pb, json);
		if (message == nullptr)
		{
			return XCode::JsonCastProtoFailure;
		}

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