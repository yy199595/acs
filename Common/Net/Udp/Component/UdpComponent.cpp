//
// Created by 64658 on 2024/10/24.
//

#include "XCode/XCode.h"
#include "Util/Tools/String.h"
#include "Message/c2s/c2s.pb.h"
#include "UdpComponent.h"
#include "Rpc/Common/Message.h"
#include "Entity/Actor/App.h"
#include "Net/Udp/Common/UdpClient.h"
#include "Server/Config/CodeConfig.h"
#include "Server/Component/ThreadComponent.h"
#include "Rpc/Component/DispatchComponent.h"
#include "Core/Thread/ThreadSync.h"

namespace acs
{
	UdpComponent::UdpComponent()
	{
		this->mActor = nullptr;
		this->mDispatch = nullptr;
	}

	bool UdpComponent::LateAwake()
	{
		this->mActor = this->GetComponent<NodeComponent>();
		this->mDispatch = this->GetComponent<DispatchComponent>();
		return true;
	}

	bool UdpComponent::StopListen()
	{
		if (this->mUdpServer == nullptr)
		{
			return false;
		}
#ifdef ONLY_MAIN_THREAD
		Asio::Code code;
		this->mUdpServer->Socket().close(code);
		if(code.value() != Asio::OK)
		{
			return false;
		}
#else
		custom::ThreadSync<bool> threadSync;
		Asio::Context& context = this->mUdpServer->GetContext();
		asio::post(context, [this, &threadSync]()
		{
			Asio::Code code;
			this->mUdpServer->Socket().close(code);
			threadSync.SetResult(code.value() == Asio::OK);
		});
		if (!threadSync.Wait())
		{
			return false;
		}
#endif
		this->mUdpServer.reset(nullptr);
		return true;
	}

	bool UdpComponent::StartListen(const acs::ListenConfig& listen)
	{
		try
		{
			Asio::Context& context = this->GetComponent<ThreadComponent>()->GetContext();
			{
				unsigned short port = listen.port;
				Asio::Context & main = this->mApp->GetContext();
				this->mUdpServer = std::make_unique<udp::Server>(context, this, port, main);
			}

			asio::post(context, [this]() { this->mUdpServer->StartReceive(); });
			return true;
		}
		catch (std::exception& e)
		{
			return false;
		}
	}

	int UdpComponent::Send(int id, rpc::Message* message) noexcept
	{
		if (message->GetType() == rpc::Type::Response)
		{
			std::string address;
			if (!message->TempHead().Del("udp", address))
			{
				return XCode::SendMessageFail;
			}
			if(!this->mUdpServer->Send(address, message))
			{
				return XCode::SendMessageFail;
			}
			return XCode::Ok;
		}

		udp::IClient* udpClient = this->GetClient(id);
		if (udpClient == nullptr)
		{
			return XCode::SendMessageFail;
		}
		udpClient->Send(message);
		return XCode::Ok;
	}

	void UdpComponent::OnMessage(rpc::Message* request, rpc::Message* response) noexcept
	{
		int code = XCode::Ok;
		switch(request->GetType())
		{
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

	int UdpComponent::OnRequest(rpc::Message* message) noexcept
	{
		int code = this->mDispatch->OnMessage(message);
		if (code != XCode::Ok)
		{
			const std::string& desc = CodeConfig::Inst()->GetDesc(code);
			LOG_ERROR("call {} code = {}", message->GetHead().GetStr(rpc::Header::func), desc);

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

	udp::Client* UdpComponent::GetClient(int id)
	{
		auto iter = this->mClients.find(id);
		if(iter != this->mClients.end())
		{
			return iter->second.get();
		}
		std::string address;
		if(!this->mActor->GetListen(id, "udp", address))
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
			Asio::Context & main = this->mApp->GetContext();
			asio_udp::endpoint remote(asio::ip::make_address(ip), port);
			Asio::Context & ctx = this->GetComponent<ThreadComponent>()->GetContext();
			std::shared_ptr<udp::Client> client = std::make_shared<udp::Client>(ctx, this, remote, main);
			{
				this->mClients.emplace(id, client);
				asio::post(ctx, [client]() { client->StartReceive(); });
			}
			return client.get();
		}
		catch (std::exception & e)
		{
			LOG_ERROR("create udp client:{} =>", address, e.what());
			return nullptr;
		}
	}
}