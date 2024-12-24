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
			char net = sender->NetType();
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

	int RouterComponent::LuaCall(lua_State* lua, int id, std::unique_ptr<rpc::Message> message)
	{
		int timeout = message->GetTimeout();
		int rpc = this->mDisComponent->BuildRpcId();
		{
			message->SetRpcId(rpc);
			int code = this->Send(id, std::move(message));
			if(code != XCode::Ok)
			{
				lua_pushinteger(lua, code);
				return 1;
			}
		}
		return this->mDisComponent->AddTask(rpc, new LuaRpcTaskSource(lua, rpc), timeout)->Await();
	}

	void RouterComponent::OnSystemUpdate() noexcept
	{
		while (!this->mLocalMessages.empty())
		{
			rpc::Message* message = this->mLocalMessages.front();
			if (this->mDisComponent->OnMessage(message) != XCode::Ok)
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
			jsonObject->Add("count", this->mLocalMessages.size());
		}
	}

	int RouterComponent::Send(int id, std::unique_ptr<rpc::Message> message)
	{
#ifdef __DEBUG__
		if(message->GetType() == rpc::Type::Request)
		{
			std::string func;
			message->GetHead().Get(rpc::Header::func, func);
			message->TempHead().Add(rpc::Header::func, func);
			message->TempHead().Add("t", help::Time::NowMil());
		}
#endif
		if(this->mApp->Equal(id))
		{
			message->SetSockId(id);
			this->mLocalMessages.emplace(message.release());
			return XCode::Ok;
		}
		ISender * sender = this->GetSender(message->GetNet());
		if(sender == nullptr)
		{
			LOG_ERROR("not find sender:{}", message->GetNet());
			return XCode::NotFoundSender;
		}
		rpc::Message * data = message.release();
		if(sender->Send(id, data) != XCode::Ok)
		{
			//LOG_ERROR("{}", data->ToString());
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

	std::unique_ptr<rpc::Message> RouterComponent::Call(int id, std::unique_ptr<rpc::Message> message)
	{
		int timeout = message->GetTimeout();
		int taskId = this->mDisComponent->BuildRpcId();
		{
			message->SetRpcId(taskId);
			if (this->Send(id, std::move(message)) != XCode::Ok)
			{
				LOG_ERROR("send to [{}] fail", id);
				return nullptr;
			}
		}
		return this->mDisComponent->AddTask(taskId, new RpcTaskSource(taskId), timeout)->Await();
	}
}