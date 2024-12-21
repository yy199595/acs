//
// Created by 64658 on 2024/10/24.
//

#include "XCode/XCode.h"
#include "Util/Tools/String.h"
#include "Message/c2s/c2s.pb.h"
#include "KcpComponent.h"
#include "Rpc/Client/Message.h"
#include "Entity/Actor/App.h"
#include "Kcp/Common/KcpClient.h"
#include "Server/Config/CodeConfig.h"
#include "Rpc/Component/DispatchComponent.h"
#include "Server/Component/ThreadComponent.h"
#include "Core/Thread/ThreadSync.h"

namespace acs
{
	KcpComponent::KcpComponent()
		: ISender(rpc::Net::Kcp)
	{
		this->mActor = nullptr;
		this->mDispatch = nullptr;
	}

	bool KcpComponent::LateAwake()
	{
		this->mActor = this->GetComponent<ActorComponent>();
		this->mDispatch = this->GetComponent<DispatchComponent>();
		return true;
	}

	bool KcpComponent::StopListen()
	{
		if (this->mKcpServer == nullptr)
		{
			return false;
		}
#ifdef ONLY_MAIN_THREAD
		Asio::Code code;
		this->mKcpServer->Socket().close(code);
		if(code.value() != Asio::OK)
		{
			return false;
		}
#else
		custom::ThreadSync<bool> threadSync;
		Asio::Executor executor = this->mKcpServer->Socket().get_executor();
		{
			asio::post(executor, [this, &threadSync]()
			{
				Asio::Code code;
				this->mKcpServer->Socket().close(code);
				threadSync.SetResult(code.value() == Asio::OK);
			});
		}
		if(!threadSync.Wait())
		{
			return false;
		}
#endif
		this->mKcpServer.reset(nullptr);
		return true;
	}

	bool KcpComponent::StartListen(const acs::ListenConfig& listen)
	{
		try
		{
			Asio::Context& context = this->GetComponent<ThreadComponent>()->GetContext();
			{
				int port = listen.Port;
				Asio::Context& io = this->mApp->GetContext();
				this->mKcpServer = std::make_unique<kcp::Server>(context, this, port, io);
			}
			asio::post(context, [this]() { this->mKcpServer->StartReceive(); });
			return true;
		}
		catch (std::exception& e)
		{
			return false;
		}
	}

	void KcpComponent::OnFrameUpdate(long long t)
	{
		auto iter = this->mClients.begin();
		for(; iter != this->mClients.end(); iter++)
		{
			iter->second->Update(t);
		}
	}

	int KcpComponent::Send(int id, rpc::Message* message)
	{
		if (message->GetType() == rpc::Type::Response)
		{
			std::string address;
			if (!message->TempHead().Del(rpc::Header::kcp_addr, address))
			{
				return XCode::SendMessageFail;
			}
			if(this->mKcpServer->Send(address, message))
			{
				return XCode::Ok;
			}
			return XCode::SendMessageFail;
		}

		kcp::IClient* udpClient = this->GetClient(id);
		if (udpClient == nullptr)
		{
			return XCode::SendMessageFail;
		}
		udpClient->Send(message);
		return XCode::Ok;
	}

	void KcpComponent::OnMessage(rpc::Message* request, rpc::Message* response)
	{
		int code = XCode::Ok;
		switch(request->GetType())
		{
			case rpc::Type::Logout:
			{
				std::string address;
				code = XCode::Failure;
				if(request->GetHead().Get(rpc::Header::kcp_addr, address))
				{
					this->mKcpServer->RemoveSession(address);
				}
				break;
			}
			case rpc::Type::Request:
				code = this->OnRequest(request);
				break;
			case rpc::Type::Response:
				code = this->mDispatch->OnMessage(request);
				break;
			default:
				code = XCode::Failure;
				break;
		}
		if(code != XCode::Ok) { delete request; }
	}

	int KcpComponent::OnRequest(rpc::Message* message)
	{
		int code = this->mDispatch->OnMessage(message);
		if (code != XCode::Ok)
		{
			const std::string& desc = CodeConfig::Inst()->GetDesc(code);
			LOG_ERROR("call {} code = {}", message->GetHead().GetStr("func"), desc);

			if (message->GetRpcId() == 0)
			{
				return XCode::Failure;
			}
			message->Body()->clear();
			message->SetType(rpc::Type::Response);
			message->GetHead().Add(rpc::Header::code, code);
			return this->Send(message->SockId(), message);
		}
		return XCode::Ok;
	}

	kcp::IClient* KcpComponent::GetClient(int id)
	{
		auto iter = this->mClients.find(id);
		if(iter != this->mClients.end())
		{
			return iter->second.get();
		}

		std::string address;
		if(!this->mActor->GetListen(id, "kcp", address))
		{
			return nullptr;
		}
		std::string ip;
		unsigned short port = 0;
		if(!help::Str::SplitAddr(address, ip, port))
		{
			return nullptr;
		}
		try
		{
			kcp::Client * udpClient = nullptr;
			Asio::Context & context = this->mApp->GetContext();
			asio_udp::endpoint remote(asio::ip::make_address(ip), port);
			std::unique_ptr<kcp::Client> client = std::make_unique<kcp::Client>(context, this, remote, context);
			{
				client->StartReceive();
				udpClient = client.get();
				this->mClients.emplace(id, std::move(client));
			}
			return udpClient;
		}
		catch(std::exception & e)
		{
			LOG_ERROR("create kcp client:{} =>", address, e.what());
			return nullptr;
		}
	}
}