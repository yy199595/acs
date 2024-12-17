
#include"InnerNetComponent.h"
#include"XCode/XCode.h"
#include"Entity/Actor/App.h"
#include"Core/System/System.h"
#include"Network/Tcp/Socket.h"
#include"DispatchComponent.h"
#include"Server/Component/ThreadComponent.h"
#include"Server/Config/CodeConfig.h"
#include"Core/Event/IEvent.h"
namespace acs
{

    InnerNetComponent::InnerNetComponent()
		: ISender(rpc::Net::Tcp), mNumPool(10000)
    {
		this->mActor = nullptr;
		this->mThread = nullptr;
		this->mDispatch = nullptr;
    }

    bool InnerNetComponent::LateAwake()
	{
		LOG_CHECK_RET_FALSE(this->mActor = this->GetComponent<ActorComponent>())
		LOG_CHECK_RET_FALSE(this->mThread = this->GetComponent<ThreadComponent>())
		LOG_CHECK_RET_FALSE(this->mDispatch = this->GetComponent<DispatchComponent>())
		return true;
	}

    void InnerNetComponent::OnMessage(rpc::Packet * message, rpc::Packet *)
	{
		int code = XCode::Ok;
		message->SetNet(rpc::Net::Tcp);
		switch (message->GetType())
		{
			case rpc::Type::Logout:
				code = XCode::Failure;
				this->StartClose(message->SockId());
				break;
			case rpc::Type::Request:
				code = this->OnRequest(message);
				break;
			case rpc::Type::Forward:
				code = this->OnForward(message);
				break;
			case rpc::Type::Response:
			{

#ifdef __DEBUG__
				std::string func;
				int code = XCode::Ok;
				long long startTime = 0;
				message->GetHead().Get("code", code);
				const rpc::Head& head = message->ConstTempHead();
				if (head.Get(rpc::Header::func, func) && head.Get("t", startTime))
				{
					if (code != XCode::Ok)
					{
						long long nowTme = help::Time::NowMil();
						const std::string error = CodeConfig::Inst()->GetDesc(code);
						LOG_WARN("({}ms) call [{}] code:{}", nowTme - startTime, func, error);
					}
				}
#endif
				code = this->mDispatch->OnMessage(message);
				break;
			}
			case rpc::Type::Client:
			case rpc::Type::Broadcast:
				code = this->mDispatch->OnMessage(message);
				break;
			default:
				code = XCode::UnKnowPacket;
			LOG_FATAL("unknown message type : {} data:{}", message->GetType(), message->ToString());
				break;
		}
		if(code != XCode::Ok)
		{
			delete message;
		}
	}

	int InnerNetComponent::OnForward(rpc::Packet* message)
	{
		int target = 0;
		int code = XCode::Ok;
		do
		{
			if(!message->GetHead().Del(rpc::Header::forward_tar, target))
			{
				code = XCode::CallArgsError;
				break;
			}
			message->SetType(rpc::Type::Request);
			message->GetHead().Add("#", message->SockId());
			if(this->Send(target, message) != XCode::Ok)
			{
				code = XCode::SendMessageFail;
				break;
			}
		}
		while(false);
		if(code != XCode::Ok)
		{
			message->SetType(rpc::Type::Response);
			return this->Send(message->SockId(), message);
		}
		return XCode::Ok;
	}

    void InnerNetComponent::OnSendFailure(int, rpc::Packet * message)
    {
        if (message->GetType() == rpc::Type::Request && message->GetRpcId() > 0)
		{
			message->SetType(rpc::Type::Response);
			message->GetHead().Add(rpc::Header::code, XCode::NetWorkError);
			if (this->mDispatch->OnMessage(message) == XCode::Ok)
			{
				return;
			}
		}
		delete message;
    }

	bool InnerNetComponent::OnListen(tcp::Socket * socket)
	{
		int id = this->mNumPool.BuildNumber();
		std::shared_ptr<rpc::InnerClient> tcpSession = std::make_shared<rpc::InnerClient>(id, this, false);
		{
			tcpSession->StartReceive(socket);
			this->mClients.emplace(id, tcpSession);
		}
		return true;
	}

    void InnerNetComponent::StartClose(int id)
    {
		auto iter = this->mClients.find(id);
		if(iter != this->mClients.end())
		{
			iter->second->Close();
			this->mClients.erase(iter);
			help::InnerLogoutEvent::Trigger(id);
		}
    }

	rpc::InnerClient * InnerNetComponent::GetClient(int id)
	{
		auto iter = this->mClients.find(id);
		if(iter != this->mClients.end())
		{
			return iter->second.get();
		}
		std::string address;
		if(!this->mActor->GetListen(id, "rpc", address))
		{
			return nullptr;
		}
		tcp::Socket* socketProxy = this->mThread->CreateSocket(address);
		if (socketProxy == nullptr)
		{
			LOG_ERROR("parse address fail : {}", address)
			return nullptr;
		}
		std::shared_ptr<rpc::InnerClient> tcpClient = std::make_shared<rpc::InnerClient>(id, this, true);
		{
			tcpClient->StartReceive(socketProxy);
			this->mClients.emplace(id, tcpClient);
		}
		return tcpClient.get();
	}

    int InnerNetComponent::Send(int id, rpc::Packet * message)
    {

        rpc::InnerClient * clientSession = this->GetClient(id);
		if(clientSession == nullptr)
		{
			LOG_ERROR("not find id : {}", id);
			return XCode::NotFoundServerRpcAddress;
		}
        return clientSession->Send(message) ? XCode::Ok : XCode::SendMessageFail;
    }

    void InnerNetComponent::OnRecord(json::w::Document &document)
    {
		std::unique_ptr<json::w::Value> data = document.AddObject("inner");
		{
			data->Add("client", (int)this->mClients.size());
		}
    }

	int InnerNetComponent::OnRequest(rpc::Packet * message)
	{
		int code = this->mDispatch->OnMessage(message);
		if (code != XCode::Ok)
		{
//			const std::string& desc = CodeConfig::Inst()->GetDesc(code);
//			LOG_ERROR("call {} code = {}", message->GetHead().GetStr(rpc::Header::func), desc);

			if (message->GetRpcId() == 0)
			{
				return XCode::DeleteData;
			}
			message->Body()->clear();
			message->SetType(rpc::Type::Response);
			message->GetHead().Add(rpc::Header::code, code);
			return this->Send(message->SockId(), message);
		}
		return XCode::Ok;
	}
}// namespace Sentry