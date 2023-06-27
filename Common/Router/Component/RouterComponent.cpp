//
// Created by leyi on 2023/6/7.
//

#include"RouterComponent.h"
#include"XCode/XCode.h"
#include"Entity/Actor/App.h"
#include"Rpc/Component/DispatchComponent.h"

namespace Tendo
{
	RouterComponent::RouterComponent()
	{
        this->mDisComponent = nullptr;
	}

	bool RouterComponent::LateAwake()
	{
		std::vector<ISender*> senders;
		this->mApp->GetComponents(senders);
		for(ISender * sender : senders)
		{
			int net = sender->NetType();
			this->mSenders.emplace(net, sender);
		}
		this->mDisComponent = this->GetComponent<DispatchComponent>();
		return this->mDisComponent != nullptr;
	}

	ISender * RouterComponent::GetSender(int net)
	{
		auto iter = this->mSenders.find(net);
		return iter == this->mSenders.end() ? nullptr : iter->second;
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
		std::string response;
		if(message->GetHead().Get("res", response))
		{
			message->GetHead().Remove("res");
		}


		int rpc = this->mDisComponent->PopTaskId();
		std::shared_ptr<LuaRpcTaskSource> luaRpcTaskSource
				= std::make_shared<LuaRpcTaskSource>(lua, rpc, response);
		{
			message->GetHead().Add("rpc", rpc);
			if (sender->Send(address, message) != XCode::Successful)
			{
				this->mDisComponent->PushTaskId(rpc);
				lua_pushinteger(lua, XCode::SendMessageFail);
				return 1;
			}
		}
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
			if(sender->Send(addr, message) != XCode::Successful)
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