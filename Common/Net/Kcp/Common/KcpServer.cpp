//
// Created by 64658 on 2024/10/26.
//

#include "KcpServer.h"
#include "Entity/Actor/App.h"
#include "Util/Tools/String.h"

namespace kcp
{
	Server::Server(asio::io_context& io, kcp::Server::Component* component, unsigned short port)
			: mContext(io), mSocket(io, asio::ip::udp::endpoint(asio::ip::udp::v4(), port)), mComponent(component)
	{

	}

	void Server::StartReceive()
	{
		this->mSocket.async_receive_from(this->mRecvBuffer.prepare(KCP_SHORT_COUNT),
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
						if (rpcPacket->OnRecvMessage(is, rpcPacket->GetProtoHead().Len) == tcp::ReadDone)
						{
							rpcPacket->SetNet(rpc::Net::Udp);
							Asio::Context& ctx = acs::App::GetContext();
							rpcPacket->TempHead().Add(rpc::Header::udp_addr, fmt::format("{}:{}", ip, port));
							asio::post(ctx, [this, msg = rpcPacket.release()] { this->mComponent->OnMessage(msg, msg); });
						}
					}
					this->mRecvBuffer.consume(size);
					asio::post(this->mContext, [this] { this->StartReceive(); });
				});
	}
}