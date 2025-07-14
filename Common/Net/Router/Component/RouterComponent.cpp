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

	int RouterComponent::LuaCall(lua_State* lua, int id, std::unique_ptr<rpc::Message> & message)
	{
		int rpcId = this->mDispatch->BuildRpcId();
		{
			message->SetRpcId(rpcId);
			int code = this->Send(id, message);
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
			std::unique_ptr<rpc::Message> & message = this->mLocalMessages.front();
			{
				this->mDispatch->OnMessage(message);
				this->mLocalMessages.pop();
			}
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

	int RouterComponent::Send(int id, std::unique_ptr<rpc::Message> & message)
	{
//#ifdef __DEBUG__
//		if(message->GetType() == rpc::type::request)
//		{
//			std::string func;
//			message->GetHead().Get(rpc::Header::func, func);
//			message->TempHead().Add(rpc::Header::func, func);
//			message->TempHead().Add("t", help::Time::NowMil());
//		}
//#endif
		int code = XCode::Ok;
		char net = message->GetNet();
		do
		{
			if(message->GetBody().size() >= rpc::INNER_RPC_BODY_MAX_LENGTH)
			{
				code = XCode::NetBigDataShutdown;
				break;
			}

			if(net == rpc::net::client)
			{
				message->SetSource(rpc::source::client);
				if(message->GetBody().size() >= rpc::OUTER_RPC_BODY_MAX_LENGTH)
				{
					code = XCode::NetBigDataShutdown;
					break;
				}
			}
			//		else if(this->mApp->Equal(id))
			//		{
			//			message->SetSockId(id);
			//			this->mLocalMessages.emplace(message.release());
			//			return XCode::Ok;
			//		}

			auto iter = this->mSenders.find(net);
			if(iter == this->mSenders.end())
			{
				LOG_ERROR("not find sender:{}", (int)net);
				code = XCode::NotFoundSender;
				break;
			}
			code = iter->second->Send(id, message);
		}
		while(false);
		if(code != XCode::Ok)
		{
			int rpcId = message->GetRpcId();
			std::string msg = message->ToString();
			LOG_WARN("({}) [send to {}:{}] ({}) => {}", code, id, (int)net, rpcId,  msg);
		}
		return code;
	}

	int RouterComponent::Send(int id, int result, std::unique_ptr<rpc::Message> & message)
	{
		if (message->GetRpcId() == 0)
		{
			return XCode::Ok;
		}
		message->SetType(rpc::type::response);
		message->GetHead().Del(rpc::Header::app_id);
		message->GetHead().Add(rpc::Header::code, result);
		return this->Send(id, message);
	}

	rpc::IInnerSender* RouterComponent::GetSender(char net)
	{
		auto iter = this->mSenders.find(net);
		return iter != this->mSenders.end() ? iter->second : nullptr;
	}

	std::unique_ptr<rpc::Message> RouterComponent::Call(int id, std::unique_ptr<rpc::Message> & message)
	{
		int rpcId = this->mDispatch->BuildRpcId();
		{
			message->SetRpcId(rpcId);
			if (this->Send(id, message) != XCode::Ok)
			{
				LOG_ERROR("send to [{}] fail", id);
				return nullptr;
			}
		}
		return this->mDispatch->BuildRpcTask<RpcTaskSource>(rpcId)->Await();
	}
}