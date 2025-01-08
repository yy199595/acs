//
// Created by mac on 2021/11/28.
//
#include "XCode/XCode.h"
#include"OuterTcpComponent.h"
#include"Rpc/Client/OuterTcpClient.h"
#include"Server/Config/CodeConfig.h"

#include"Entity/Actor/App.h"
#include"Core/Event/IEvent.h"
#include "Core/System/System.h"
#include "Gate/Component/GateComponent.h"

namespace acs
{
	OuterTcpComponent::OuterTcpComponent()
	{
		this->mGate = nullptr;
		this->mWaitCount = 0;
		this->mMaxConnectCount = 500;
	}

	bool OuterTcpComponent::LateAwake()
	{
		std::unique_ptr<json::r::Value> jsonObject;
		if(ServerConfig::Inst()->Get("connect", jsonObject))
		{
			jsonObject->Get("outer", this->mMaxConnectCount);
		}
		help::PlayerLoginEvent::Add(this, &OuterTcpComponent::OnPlayerLogin);
		help::PlayerLogoutEvent::Add(this, &OuterTcpComponent::OnPlayerLogout);
		LOG_CHECK_RET_FALSE(this->mGate = this->GetComponent<GateComponent>());
		return true;
	}

	void OuterTcpComponent::OnPlayerLogin(long long userId, int sockId)
	{
		auto iter = this->mGateClientMap.find(sockId);
		if(iter != this->mGateClientMap.end())
		{
			iter->second->BindPlayer(userId);
#if __DEBUG__
			os::SystemInfo systemInfo;
			os::System::GetSystemInfo(systemInfo);
			double mb = (double )systemInfo.use_memory / (1024 * 1024.0f);
			LOG_DEBUG("[{}] user({}) login ok => {:.3f}MB", this->mGateClientMap.size(), userId, mb);
#endif
		}
	}

	void OuterTcpComponent::OnPlayerLogout(long long userId, int sockId)
	{
		auto iter = this->mGateClientMap.find(sockId);
		if(iter != this->mGateClientMap.end())
		{
			this->mGateClientMap.erase(iter);
		}
	}

	void OuterTcpComponent::OnMessage(rpc::Message * message, rpc::Message *) noexcept
	{
		int code = this->mGate->OnMessage(message);
		if(code != XCode::Ok)
		{
			int sockId = message->SockId();
			this->StartClose(sockId, code);
		}
	}

	int OuterTcpComponent::Send(int id, rpc::Message * message) noexcept
	{
		if (message->GetType() == rpc::Type::Response)
		{
			--this->mWaitCount;
		}
		auto iter = this->mGateClientMap.find(id);
		if (iter == this->mGateClientMap.end())
		{
			return XCode::SendMessageFail;
		}
		iter->second->Send(message);
		return XCode::Ok;
	}

	bool OuterTcpComponent::OnListen(tcp::Socket * socket) noexcept
	{
		if (this->mApp->GetStatus() < ServerStatus::Start)
		{
			return false;
		}

		int sockId = this->mSocketPool.BuildNumber();
		Asio::Context & io = this->mApp->GetContext();
		std::shared_ptr<rpc::OuterTcpClient> outerNetClient = std::make_shared<rpc::OuterTcpClient>(sockId, this, io);
		{
			outerNetClient->StartReceive(socket);
			this->mGateClientMap.emplace(sockId, outerNetClient);
		}
		//LOG_DEBUG("[{}] connect gate server count:{}", socket->GetAddress(), this->mGateClientMap.size())
		return true;
	}

	void OuterTcpComponent::OnClientError(int id, int code)
	{
		auto iter = this->mGateClientMap.find(id);
		if(iter != this->mGateClientMap.end())
		{
			long long playerId = iter->second->GetPlayerId();
			help::PlayerLogoutEvent::Trigger(playerId, id);
		}
		LOG_DEBUG("remove client({}) count:{}", id, this->mGateClientMap.size());
	}

	void OuterTcpComponent::OnSendFailure(int id, rpc::Message* message)
	{
		int sockId = 0;
		message->GetHead().Add(rpc::Header::code, XCode::SendMessageFail);
		if(message->GetType() == rpc::Type::Request && message->GetRpcId() > 0)
		{
			if(message->GetHead().Get(rpc::Header::sock_id, sockId))
			{
				this->Send(sockId, message);
				return;
			}
		}
		delete message;
	}

	void OuterTcpComponent::StartClose(int id, int code)
	{
		auto iter = this->mGateClientMap.find(id);
		if(iter != this->mGateClientMap.end())
		{
			iter->second->Stop();
			long long playerId = iter->second->GetPlayerId();
			help::PlayerLogoutEvent::Trigger(playerId, id);
		}
	}

    void OuterTcpComponent::OnRecord(json::w::Document &document)
    {
		std::unique_ptr<json::w::Value> data = document.AddObject("outer");
		{
			data->Add("wait", this->mWaitCount);
			data->Add("client", this->mGateClientMap.size());
		}
    }

	void OuterTcpComponent::Broadcast(rpc::Message * message) noexcept
	{
		message->SetType(rpc::Type::Request);
		for(auto iter = this->mGateClientMap.begin(); iter != this->mGateClientMap.end(); iter++)
		{
			if(iter->second->GetPlayerId() == 0)
			{
				continue;
			}
			std::unique_ptr<rpc::Message> broadCastMessage = message->Clone();
			{
				iter->second->Send(broadCastMessage.release());
			}
		}
		delete message;
	}
}