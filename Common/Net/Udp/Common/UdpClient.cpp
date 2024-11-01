//
// Created by 64658 on 2024/10/24.
//

#include "UdpClient.h"
#include "Util/Tools/String.h"
#include "Entity/Actor/App.h"
#include "Log/Common/CommonLogDef.h"

namespace udp
{
	Client::Client(asio::io_context& io,
			  udp::Client::Component* component, asio_udp::endpoint & remote)
			: mContext(io), mComponent(component), mSocket(io, asio_udp::endpoint(asio_udp::v4(), 0)),
			 mRemoteEndpoint(remote)
	{

	}

	void Client::Send(tcp::IProto* message)
	{
		asio::post(this->mContext, [this, message]()
		{
			std::ostream stream(&this->mSendBuffer);
			int length = message->OnSendMessage(stream);
			this->mSocket.async_send_to(this->mSendBuffer.data(), this->mRemoteEndpoint,
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
	}

	void Client::OnSendMessage()
	{

	}

	void Client::OnSendMessage(const Asio::Code& code)
	{

	}

	void Client::StartReceive()
	{
		this->mSocket.async_receive_from(this->mRecvBuffer.prepare(udp::BUFFER_COUNT),
				this->mLocalEndpoint, [this](const asio::error_code& code, size_t size)
				{
					if (code.value() == Asio::OK)
					{
						this->mRecvBuffer.commit(size);
						std::istream is(&this->mRecvBuffer);
						unsigned short port = this->mLocalEndpoint.port();
						std::string ip = this->mLocalEndpoint.address().to_string();
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
									asio::post(ctx, [this, msg = rpcPacket.release()] { this->mComponent->OnMessage(msg, msg); });
								}
							}
						}
						this->mRecvBuffer.consume(size);
					}
					asio::post(this->mSocket.get_executor(), [this] { this->StartReceive(); });
				});
	}
}