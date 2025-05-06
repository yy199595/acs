//
// Created by leyi on 2023/6/7.
//

#include "RouterComponent.h"
#include "XCode/XCode.h"
#include "Lua/Lib/Lib.h"
#include "Entity/Actor/App.h"
#include "Rpc/Component/DispatchComponent.h"
#include "Core/Singleton/Singleton.h"

namespace acs
{
	RouterComponent::RouterComponent()
	{
        this->mDispatch = nullptr;
	}

	bool RouterComponent::Awake()
	{
		LuaCCModuleRegister::Add([](Lua::CCModule & ccModule) {
			ccModule.Open("core.router", lua::lib::luaopen_lrouter);
		});
		return true;
	}

	bool RouterComponent::LateAwake()
	{
		std::vector<rpc::IInnerSender*> senders;
		this->mApp->GetComponents(senders);
		for(rpc::IInnerSender * sender : senders)
		{
			char net = sender->GetNet();
			this->mSenders.emplace(net, sender);
		}
		LOG_CHECK_RET_FALSE(!this->mSenders.empty())
		LOG_CHECK_RET_FALSE(this->mDispatch = this->GetComponent<DispatchComponent>())
		return true;
	}

	int RouterComponent::LuaCall(lua_State* lua, int id, std::unique_ptr<rpc::Message> message)
	{
		int rpcId = this->mDispatch->BuildRpcId();
		{
			message->SetRpcId(rpcId);
			int code = this->Send(id, std::move(message));
			if(code != XCode::Ok)
			{
				lua_pushinteger(lua, code);
				return 1;
			}
		}
		return this->mDispatch->AddTask(new LuaRpcTaskSource(lua, rpcId))->Await();
	}

	void RouterComponent::OnSystemUpdate() noexcept
	{
		while (!this->mLocalMessages.empty())
		{
			rpc::Message* message = this->mLocalMessages.front();
			if (this->mDispatch->OnMessage(message) != XCode::Ok)
			{
				delete message;
			}
			this->mLocalMessages.pop();
		}
	}

	void RouterComponent::OnRecord(json::w::Document& document)
	{
		auto jsonObject = document.AddObject("router");
		{
			jsonObject->Add("sender", this->mSenders.size());
			jsonObject->Add("local", this->mLocalMessages.size());
		}
	}

	int RouterComponent::Send(int id, std::unique_ptr<rpc::Message> message)
	{
//#ifdef __DEBUG__
//		if(message->GetType() == rpc::Type::Request)
//		{
//			std::string func;
//			message->GetHead().Get(rpc::Header::func, func);
//			message->TempHead().Add(rpc::Header::func, func);
//			message->TempHead().Add("t", help::Time::NowMil());
//		}
//#endif
		char net = message->GetNet();
		if(net == rpc::Net::Client)
		{
			message->SetSource(rpc::Source::Client);
		}
		else if(this->mApp->Equal(id))
		{
			message->SetSockId(id);
			this->mLocalMessages.emplace(message.release());
			return XCode::Ok;
		}
		auto iter = this->mSenders.find(net);
		if(iter == this->mSenders.end())
		{
			LOG_ERROR("not find sender:{}", net);
			return XCode::NotFoundSender;
		}

		rpc::Message * data = message.release();
		if(iter->second->Send(id, data) != XCode::Ok)
		{
			delete data;
			return XCode::SendMessageFail;
		}
		return XCode::Ok;
	}

	int RouterComponent::Send(int id, int result, rpc::Message * message)
	{
		if (message->GetRpcId() == 0)
		{
			delete message;
			return XCode::Ok;
		}
		message->SetType(rpc::Type::Response);
		message->GetHead().Del(rpc::Header::app_id);
		message->GetHead().Add(rpc::Header::code, result);
		return this->Send(id, std::unique_ptr<rpc::Message>(message));
	}

	rpc::IInnerSender* RouterComponent::GetSender(char net)
	{
		auto iter = this->mSenders.find(net);
		return iter != this->mSenders.end() ? iter->second : nullptr;
	}

	std::unique_ptr<rpc::Message> RouterComponent::Call(int id, std::unique_ptr<rpc::Message> message)
	{
		int rpcId = this->mDispatch->BuildRpcId();
		{
			message->SetRpcId(rpcId);
			if (this->Send(id, std::move(message)) != XCode::Ok)
			{
				LOG_ERROR("send to [{}] fail", id);
				return nullptr;
			}
		}
		return this->mDispatch->BuildRpcTask<RpcTaskSource>(rpcId)->Await();
	}
}