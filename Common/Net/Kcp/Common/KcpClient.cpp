//
// Created by 64658 on 2024/10/24.
//

#include "KcpClient.h"
#include "Util/Tools/String.h"
#include "Entity/Actor/App.h"
#include "Log/Common/CommonLogDef.h"
#include "Util/Tools/TimeHelper.h"
namespace kcp
{
	Client::Client(asio::io_context& io,
			kcp::Client::Component* component, asio_udp::endpoint & remote)
			: mContext(io), mComponent(component), mSocket(io, asio_udp::endpoint(asio_udp::v4(), 0)),
			 mRemoteEndpoint(remote), mTimer(io), mSendStream(&mSendBuffer)
	{
		this->mKcp = ikcp_create(0x01, this);
		this->mKcp->output = kcp::OnKcpSend;
		ikcp_wndsize(this->mKcp, kcp::BUFFER_COUNT, kcp::BUFFER_COUNT);
		ikcp_nodelay(this->mKcp, 1, kcp::REFRESH_INTERVAL, kcp::RESEND, 1);
	}

	void Client::Send(const char* buf, int len)
	{
		asio::error_code code;
		this->mSocket.send_to(asio::buffer(buf, len), this->mRemoteEndpoint, 0, code);
		if(code.value() != Asio::OK)
		{
			unsigned short port = this->mLocalEndpoint.port();
			const std::string ip = this->mLocalEndpoint.address().to_string();
			LOG_ERROR("kcp send [{}:{}] =>{}", ip, port, code.message());
		}
	}

	void Client::Send(tcp::IProto* message)
	{
		asio::post(this->mContext, [this, message]()
		{
			message->OnSendMessage(this->mSendStream);
			const int len = (int)this->mSendBuffer.size();
			const char* msg = asio::buffer_cast<const char*>(this->mSendBuffer.data());
			{
				ikcp_send(this->mKcp, msg, len);
				this->mSendBuffer.consume(len);
			}
			delete message;
		});
	}

	void Client::Update(long long ms)
	{
		ikcp_update(this->mKcp, (IUINT32)ms);
	}

	void Client::OnSendMessage()
	{

	}

	void Client::OnSendMessage(const Asio::Code& code)
	{

	}

	void Client::StartReceive()
	{
		this->mSocket.async_receive_from(this->mReceiveBuffer.prepare(kcp::BUFFER_COUNT),
				this->mLocalEndpoint, [this](const asio::error_code& code, size_t size)
				{
					if (code.value() != Asio::OK)
					{
						CONSOLE_LOG_ERROR("code:{}", code.message())
						return;
					}
					this->mReceiveBuffer.commit(size);
					unsigned short port = this->mLocalEndpoint.port();
					std::string ip = this->mLocalEndpoint.address().to_string();

					const char * msg = asio::buffer_cast<const char *>(this->mReceiveBuffer.data());

					ikcp_input(this->mKcp, msg, (int)size);
					int messageLen = ikcp_recv(this->mKcp, this->mDecodeBuffer.data(), kcp::BUFFER_COUNT);
					//CONSOLE_LOG_DEBUG("client receive message : {}", messageLen);
					if(messageLen > 0)
					{
						const std::string address = fmt::format("{}:{}", ip, port);
						std::unique_ptr<rpc::Packet> rpcPacket = std::make_unique<rpc::Packet>();
						{
							if(rpcPacket->Decode(this->mDecodeBuffer.data(), messageLen))
							{
								rpcPacket->SetNet(rpc::Net::Kcp);
								rpcPacket->TempHead().Add(rpc::Header::kcp_addr, address);
								asio::post(acs::App::GetContext(), [this, msg = rpcPacket.release()] {
									this->mComponent->OnMessage(msg, nullptr);
								});
							}
						}
					}
					this->mReceiveBuffer.consume(size);
					asio::post(this->mContext, [this] { this->StartReceive(); });
				});
	}
}