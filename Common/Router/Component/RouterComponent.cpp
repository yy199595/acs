//
// Created by leyi on 2023/6/7.
//

#include"RouterComponent.h"
#include"XCode/XCode.h"
#include"Entity/Actor/ServerActor.h"
#include"Http/Component/HttpComponent.h"
#include"Redis/Component/RedisComponent.h"
#include"Rpc/Component/InnerNetComponent.h"
#include"Rpc/Component/DispatchComponent.h"

namespace Tendo
{
	RouterComponent::RouterComponent()
	{
		this->mNetComponent = nullptr;
		this->mHttpComponent = nullptr;
		this->mRedisComponent = nullptr;
	}

	bool RouterComponent::LateAwake()
	{
		this->mHttpComponent = this->GetComponent<HttpComponent>();
		this->mRedisComponent = this->GetComponent<RedisComponent>();
		this->mNetComponent = this->GetComponent<InnerNetComponent>();
		this->mDisComponent = this->GetComponent<DispatchComponent>();
		return this->mDisComponent != nullptr;
	}

	ISender * RouterComponent::GetSender(int net)
	{
		switch (net)
		{
			case Msg::Net::Tcp:
				return this->mNetComponent;
			case Msg::Net::Http:
				return this->mHttpComponent;
			case Msg::Net::Redis:
				//return this->mRedisComponent;
				break;
		}
		return nullptr;
	}

	int RouterComponent::LuaCall(lua_State* lua,
			const std::string& address, const std::shared_ptr<Msg::Packet>& message)
	{
		ISender * sender = this->GetSender(message->GetNet());
		if(sender == nullptr)
		{
			lua_pushinteger(lua, XCode::NotFoundActorAddress);
			return 1;
		}

		int rpc = this->mDisComponent->PopTaskId();
		message->GetHead().Add("rpc", rpc);
		if(!sender->Send(address, message))
		{
			lua_pushinteger(lua, XCode::SendMessageFail);
			return 1;
		}
		const std::string & response = message->From();
		std::shared_ptr<LuaRpcTaskSource> luaRpcTaskSource
				= std::make_shared<LuaRpcTaskSource>(lua, rpc, response);
		return this->mDisComponent->AddTask(rpc, luaRpcTaskSource)->Await();
	}

	int RouterComponent::Send(const std::string &addr, const std::shared_ptr<Msg::Packet>& message)
	{
		ISender * sender = this->GetSender(message->GetNet());
		if(sender == nullptr || addr.empty())
		{
			return XCode::NotFoundActorAddress;
		}
		return sender->Send(addr, message);
	}

	int RouterComponent::Send(const std::string& addr, int code, const std::shared_ptr<Msg::Packet>& message)
	{
		if(!message->GetHead().Has("rpc"))
		{
			return XCode::Successful;
		}
		message->SetType(Msg::Type::Response);
		message->GetHead().Add("code", code);
		ISender * sender = this->GetSender(message->GetNet());
		if(sender == nullptr)
		{
			LOG_ERROR("not find sender : " << message->GetNet());
			return XCode::SendMessageFail;
		}
		return sender->Send(addr, message);
	}

	std::shared_ptr<Msg::Packet> RouterComponent::Call(
			const std::string &addr, const std::shared_ptr<Msg::Packet>& message)
	{
		std::shared_ptr<Msg::Packet> response;
		ISender * sender = this->GetSender(message->GetNet());
		if(sender == nullptr || addr.empty())
		{
			response = std::make_shared<Msg::Packet>();
			response->GetHead().Add("code", XCode::NotFoundActorAddress);
		}
		else
		{
			int taskId = this->mDisComponent->PopTaskId();

			message->GetHead().Add("rpc", taskId);
			if(!sender->Send(addr, message))
			{
				this->mDisComponent->PushTaskId(taskId);
				response = std::make_shared<Msg::Packet>();
				response->GetHead().Add("code", XCode::SendMessageFail);
			}
			else
			{
				std::shared_ptr<RpcTaskSource> taskSource =
						std::make_shared<RpcTaskSource>(taskId);
				response = this->mDisComponent->AddTask(taskId, taskSource)->Await();
			}
		}
		return response;
	}
}