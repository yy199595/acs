//
// Created by leyi on 2023/6/7.
//

#include"RouterComponent.h"
#include"XCode/XCode.h"
#include"Entity/Actor/App.h"
#include"Rpc/Component/DispatchComponent.h"

namespace acs
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

	ISender * RouterComponent::GetSender(char net)
	{
		auto iter = this->mSenders.find(net);
		return iter == this->mSenders.end() ? nullptr : iter->second;
	}

	int RouterComponent::LuaCall(lua_State* lua, int id, std::unique_ptr<rpc::Packet> message)
	{
		ISender * sender = this->GetSender(message->GetNet());
		if(sender == nullptr)
		{
			luaL_error(lua, "not found server:%d", (int)message->GetNet());
			lua_pushinteger(lua, XCode::NotFoundSender);
			return 1;
		}
		int timeout = message->GetTimeout();
		int rpc = this->mDisComponent->BuildRpcId();
		{
			message->SetRpcId(rpc);
			int code = sender->Send(id, message.release());
			if(code != XCode::Ok)
			{
				lua_pushinteger(lua, code);
				return 1;
			}
		}
		return this->mDisComponent->AddTask(rpc, new LuaRpcTaskSource(lua, rpc), timeout)->Await();
	}

	int RouterComponent::Send(int id, std::unique_ptr<rpc::Packet> message)
	{
		ISender * sender = this->GetSender(message->GetNet());
		if(sender == nullptr)
		{
			return XCode::NotFoundSender;
		}
		rpc::Packet * data = message.release();
		if(sender->Send(id, data) != XCode::Ok)
		{
			delete data;
			LOG_ERROR("send to {} fail", id);
			return XCode::SendMessageFail;
		}
		return XCode::Ok;
	}

	int RouterComponent::Send(int id, int result, rpc::Packet * message)
	{
		int code = XCode::Ok;
		do
		{
			if(message->GetRpcId() == 0)
			{
				code = XCode::DeleteData;
				break;
			}
			const char net = message->GetNet();
			ISender * sender = this->GetSender(net);
			if(sender == nullptr)
			{
				code = XCode::NotFoundSender;
				LOG_ERROR("not find sender {}", (int)net);
				break;
			}
			message->GetHead().Del("app");
			message->SetType(rpc::Type::Response);
			message->GetHead().Add(rpc::Header::code, result);
			code = sender->Send(id, message);
		}
		while(false);
		if(code != XCode::Ok)
		{
			delete message;
		}
		return code;
	}

	rpc::Packet * RouterComponent::Call(int id, std::unique_ptr<rpc::Packet> message)
	{
		ISender* sender = this->GetSender(message->GetNet());
		if (sender == nullptr)
		{
			LOG_ERROR("send to [{}] fail", id);
			return nullptr;
		}
		int timeout = message->GetTimeout();
		int taskId = this->mDisComponent->BuildRpcId();

		message->SetRpcId(taskId);
		if (sender->Send(id, message.release()) != XCode::Ok)
		{
			LOG_ERROR("send to [{}] fail", id);
			return nullptr;
		}
		return this->mDisComponent->AddTask(taskId, new RpcTaskSource(taskId), timeout)->Await();
	}
}