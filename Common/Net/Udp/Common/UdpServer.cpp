//
// Created by 64658 on 2024/10/26.
//

#include "UdpServer.h"
#include "Entity/Actor/App.h"
#include "Util/Tools/String.h"

namespace udp
{
	Server::Server(asio::io_context& io, udp::Server::Component* component, unsigned short port)
			: mContext(io), mSocket(io, asio::ip::udp::endpoint(asio::ip::udp::v4(), port)), mComponent(component)
	{

	}

	void Server::StartReceive()
	{
		this->mSocket.async_receive_from(this->mRecvBuffer.prepare(udp::BUFFER_COUNT),
				this->mSenderPoint, [this](const asio::error_code& code, size_t size)
				{
					if (code.value() != Asio::OK)
					{
						CONSOLE_LOG_ERROR("code:{}", code.message())
						return;
					}
					this->mRecvBuffer.commit(size);
					std::istream is(&this->mRecvBuffer);
					unsigned short port = this->mSenderPoint.port();
					std::string ip = this->mSenderPoint.address().to_string();
					//CONSOLE_LOG_ERROR("receive ({}:{}) size={}", ip, port, size)
					std::unique_ptr<rpc::Packet> rpcPacket = std::make_unique<rpc::Packet>();
					{
						tcp::Data::Read(is, rpcPacket->GetProtoHead());
						if((size - rpc::RPC_PACK_HEAD_LEN) == rpcPacket->GetProtoHead().Len)
						{
							if (rpcPacket->OnRecvMessage(is, rpcPacket->GetProtoHead().Len) == tcp::ReadDone)
							{
								rpcPacket->SetNet(rpc::Net::Udp);
								Asio::Context& ctx = acs::App::GetContext();
								rpcPacket->TempHead().Add(rpc::Header::udp_addr, fmt::format("{}:{}", ip, port));
								asio::post(ctx, [this, msg = rpcPacket.release()] { this->mComponent->OnMessage(msg, nullptr); });
							}
						}
					}
					this->mRecvBuffer.consume(size);
					asio::post(this->mContext, [this] { this->StartReceive(); });
				});
	}

	bool Server::Send(const std::string& addr, tcp::IProto* message)
	{
		std::string ip;
		unsigned short port = 0;
		if(!help::Str::SplitAddr(addr, ip, port))
		{
			return false;
		}
		return this->Send(ip, port, message);
	}

	bool Server::Send(const std::string & ip, unsigned short port, tcp::IProto* message)
	{
		asio::post(this->mContext, [this, message, ip, port]()
		{
			std::ostream stream(&this->mSendBuffer);
			int length = message->OnSendMessage(stream);
			asio::ip::udp::endpoint endpoint(asio::ip::make_address(ip), port);
			this->mSocket.async_send_to(this->mSendBuffer.data(), endpoint,
					[this, length, message](const asio::error_code& code, size_t size)
					{
						if (code.value() != Asio::OK)
						{
							return;
						}
						this->mSendBuffer.consume(size);
						//unsigned short port = this->mRemoteEndpoint.port();
						//std::string ip = this->mRemoteEndpoint.address().to_string();
						//CONSOLE_LOG_ERROR("send:({}:{}) size:{}", ip, port, size);
					});
		});
		return true;
	}
}