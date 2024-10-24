//
// Created by 64658 on 2024/10/24.
//

#include "XCode/XCode.h"
#include "Message/c2s/c2s.pb.h"
#include "UdpListenComponent.h"
#include "Rpc/Client/Message.h"
#include "Entity/Actor/App.h"
#include "Server/Component/ThreadComponent.h"
#include "Rpc/Component/DispatchComponent.h"
namespace acs
{
	UdpListenComponent::UdpListenComponent()
		: ISender(rpc::Net::Udp)
	{
		this->mPort = 0;
		this->mDispatch = nullptr;
	}

	bool UdpListenComponent::LateAwake()
	{
		try
		{
			std::unique_ptr<json::r::Value> jsonObject;
			const ServerConfig & config = this->mApp->Config();
			LOG_CHECK_RET_FALSE(config.Get("listen", jsonObject));
			LOG_CHECK_RET_FALSE(jsonObject->Get("udp",jsonObject));
			LOG_CHECK_RET_FALSE(jsonObject->Get("port",this->mPort));
			LOG_CHECK_RET_FALSE(this->mDispatch = this->GetComponent<DispatchComponent>());
			Asio::Context & context = this->GetComponent<ThreadComponent>()->GetContext();
			{
				asio_udp::endpoint endpoint(asio_udp::v4(), this->mPort);
				this->mSocket = std::make_unique<asio_udp::socket>(context);
				this->mSocket->open(asio_udp ::v4());
				this->mSocket->bind(endpoint);
			}
			LOG_INFO("listen [udp:{}] ok", this->mPort);
			asio::post(context, [this]{ this->StartReceive(); });
			return true;
		}
		catch(std::exception & e)
		{
			LOG_ERROR("listen udp:{} =>{}", this->mPort, e.what());
			return false;
		}
	}

	void UdpListenComponent::Complete()
	{
		asio::any_io_executor executor = this->mSocket->get_executor();
		std::unique_ptr<udp::Client> client = std::make_unique<udp::Client>(executor);
		{
			client->Init("127.0.0.1:7787");
			rpc::Packet * rpcPacket = new rpc::Packet();

			c2s::chat::request request;
			request.set_user_id(10004);
			request.set_message("hello");

			rpcPacket->WriteMessage(&request);
			rpcPacket->GetHead().Add("func", "ChatSystem.Chat");

			client->Send(rpcPacket);
		}

		int id = this->mNumberPool.BuildNumber();
		this->mClients.emplace(id, std::move(client));
	}

	void UdpListenComponent::StartReceive()
	{
		this->mSocket->async_receive_from(this->mReceiveBuffer.prepare(1024 * 1024),
				this->mRemotePoint, [this](const asio::error_code & code, size_t size)
		{
			this->mReceiveBuffer.commit(size);
			CONSOLE_LOG_ERROR("size={}", size);
			asio::any_io_executor executor = this->mSocket->get_executor();
			if(code.value() == Asio::OK)
			{
				std::istream is(&this->mReceiveBuffer);
				std::unique_ptr<rpc::Packet> rpcPacket = std::make_unique<rpc::Packet>();
				if(rpcPacket->OnRecvMessage(is, size) == tcp::ReadDone)
				{
					rpcPacket->SetNet(rpc::Net::Udp);
					rpcPacket->SetType(rpc::Type::Request);
					rpcPacket->TempHead().Add("addr", this->mRemotePoint.address().to_string());
					asio::post(executor, [this, msg = rpcPacket.release()] { this->OnMessage(msg); });
				}
			}
			this->mReceiveBuffer.consume(size);
			asio::post(executor, [this]{ this->StartReceive(); });
		});
	}

	int UdpListenComponent::Send(int id, rpc::Packet* message)
	{
		auto iter = this->mClients.find(id);
		if(iter != this->mClients.end())
		{
			iter->second->Send(message);
			return true;
		}
		return XCode::Ok;
	}

	void UdpListenComponent::OnMessage(rpc::Packet * message)
	{
		int code = XCode::Ok;
		switch(message->GetType())
		{
			case rpc::Type::Auth:
				code = this->OnLogin(message);
				break;
			case rpc::Type::Logout:
				code = this->OnLogout(message);
				break;
			case rpc::Type::Request:
			case rpc::Type::Response:
				code = this->mDispatch->OnMessage(message);
				break;
			default:
				code = XCode::Failure;
				break;
		}
		if(code != XCode::Ok) { delete message; }
	}

	int UdpListenComponent::OnLogin(rpc::Packet* message)
	{
		return XCode::Ok;
	}

	int UdpListenComponent::OnLogout(rpc::Packet* message)
	{
		return XCode::Ok;
	}
}