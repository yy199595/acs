//
// Created by 64658 on 2024/10/24.
//

#include "XCode/XCode.h"
#include "Util/Tools/String.h"
#include "Message/c2s/c2s.pb.h"
#include "KcpComponent.h"
#include "Rpc/Common/Message.h"
#include "Entity/Actor/App.h"
#include "Kcp/Common/KcpClient.h"
#include "Server/Config/CodeConfig.h"
#include "Rpc/Component/DispatchComponent.h"
#include "Server/Component/ThreadComponent.h"
#include "Core/Thread/ThreadSync.h"

namespace acs
{
	KcpComponent::KcpComponent()
	{
		this->mActor = nullptr;
		this->mDispatch = nullptr;
	}

	bool KcpComponent::LateAwake()
	{
		this->mActor = this->GetComponent<NodeComponent>();
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
		return true;
	}

	bool KcpComponent::StartListen(const acs::ListenConfig& listen)
	{
		try
		{
			Asio::Context& context = this->GetComponent<ThreadComponent>()->GetContext();
			{
				unsigned int port = listen.port;
				Asio::Context& io = this->mApp->GetContext();
				this->mKcpServer = std::make_shared<kcp::Server>(context, this, port, io);
			}
			asio::post(context, [this] { this->mKcpServer->Start(); });
			return true;
		}
		catch (std::exception& e)
		{
			return false;
		}
	}

	void KcpComponent::OnFrameUpdate(long long t) noexcept
	{
		auto iter = this->mClients.begin();
		for(; iter != this->mClients.end(); iter++)
		{
			iter->second->Update(t);
		}
	}

	int KcpComponent::Send(int id, std::unique_ptr<rpc::Message> & message) noexcept
	{
		if (message->GetType() == rpc::type::response)
		{
			std::string address;
			if (!message->TempHead().Del(rpc::Header::from_addr, address))
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

	void KcpComponent::OnMessage(rpc::Message* req, rpc::Message* response) noexcept
	{
		std::unique_ptr<rpc::Message> request(req);
		switch(request->GetType())
		{
			case rpc::type::logout:
			{
				std::string address;
				if(request->GetHead().Get(rpc::Header::from_addr, address))
				{
					this->mKcpServer->RemoveSession(address);
				}
				break;
			}
			case rpc::type::request:
				this->OnRequest(request);
				break;
			case rpc::type::response:
				this->mDispatch->OnMessage(request);
				break;
			default:
				break;
		}
	}

	int KcpComponent::OnRequest(std::unique_ptr<rpc::Message> & message)
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
			message->SetType(rpc::type::response);
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
			std::shared_ptr<kcp::Client> client = std::make_shared<kcp::Client>(context, this, remote, context);
			{
				client->StartReceive();
				udpClient = client.get();
				this->mClients.emplace(id, client);
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