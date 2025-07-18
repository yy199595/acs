//
// Created by 64658 on 2024/10/24.
//

#include "KcpClient.h"
#include <sstream>
#include "Util/Tools/String.h"
#include "Log/Common/CommonLogDef.h"
#include "Util/Tools/TimeHelper.h"
namespace kcp
{
	Client::Client(asio::io_context& io,
			kcp::Client::Component* component, asio_udp::endpoint & remote, Asio::Context & main)
			: mContext(io), mComponent(component), mSocket(io, asio_udp::endpoint(asio_udp::v4(), 0)),
			 mRemoteEndpoint(remote), mTimer(io), mSendStream(&mSendBuffer), mMainContext(main)
	{
		this->mKcp = ikcp_create(0x01, this);
		this->mKcp->output = kcp::OnKcpSend;
		ikcp_wndsize(this->mKcp, kcp::BUFFER_COUNT, kcp::BUFFER_COUNT);
		ikcp_nodelay(this->mKcp, 1, kcp::REFRESH_INTERVAL, kcp::RESEND, 1);
	}

	void Client::Send(const char* buf, int len)
	{
		std::shared_ptr<Client> self = this->shared_from_this();
		this->mSocket.async_send(asio::buffer(buf, len), [self](const asio::error_code & code, size_t)
		{
			if(code.value() != Asio::OK)
			{
				unsigned short port = self->mLocalEndpoint.port();
				const std::string ip = self->mLocalEndpoint.address().to_string();
				LOG_ERROR("kcp send [{}:{}] =>{}", ip, port, code.message());
			}
		});
		//asio::error_code code;
		//this->mSocket.send_to(asio::buffer(buf, len), this->mRemoteEndpoint, 0, code);
	}

	void Client::Send(std::unique_ptr<rpc::Message> & message)
	{
		std::shared_ptr<Client> self = this->shared_from_this();
		asio::post(this->mContext, [self, req = message.release()]()
		{
			std::unique_ptr<rpc::Message> message(req);
			message->OnSendMessage(self->mSendStream);
			const int len = (int)self->mSendBuffer.size();
			const char* msg = asio::buffer_cast<const char*>(self->mSendBuffer.data());
			{
				ikcp_send(self->mKcp, msg, len);
				self->mSendBuffer.consume(len);
			}
		});
	}

	void Client::Update(long long ms)
	{
		ikcp_update(this->mKcp, (IUINT32)ms);
	}

	void Client::StartReceive()
	{
		std::shared_ptr<Client> self = this->shared_from_this();
		auto callback = [self](const asio::error_code& code, size_t size)
		{
			if (code.value() != Asio::OK)
			{
				CONSOLE_LOG_ERROR("code:{}", code.message())
				return;
			}
			self->mReceiveBuffer.commit(size);
			unsigned short port = self->mLocalEndpoint.port();
			std::string ip = self->mLocalEndpoint.address().to_string();

			const char* msg = asio::buffer_cast<const char*>(self->mReceiveBuffer.data());

			ikcp_input(self->mKcp, msg, (int)size);
			self->mDecodeBuffer.resize(kcp::BUFFER_COUNT);
			char * buffer = (char *)self->mDecodeBuffer.data();
			int messageLen = ikcp_recv(self->mKcp, buffer, kcp::BUFFER_COUNT);
			if (messageLen > 0)
			{
				const std::string address = fmt::format("{}:{}", ip, port);
				self->OnReceive(address, self->mDecodeBuffer.data(), messageLen);
			}
			self->mReceiveBuffer.consume(size);
			asio::post(self->mContext, [self] { self->StartReceive(); });
		};
		auto buffer = this->mReceiveBuffer.prepare(kcp::BUFFER_COUNT);
		this->mSocket.async_receive_from(buffer, this->mLocalEndpoint, callback);
	}

	void Client::OnReceive(const std::string& address, const std::string & buf, size_t size)
	{
		std::stringstream buffer(buf);
		std::unique_ptr<rpc::Message> rpcPacket = std::make_unique<rpc::Message>();
		{
			rpc::ProtoHead protoHead;
			tcp::Data::Read(buffer, protoHead);
			if(rpcPacket->OnRecvMessage(buffer, size) == 0)
			{
				rpcPacket->Init(protoHead);
				rpcPacket->SetNet(rpc::net::kcp);
				rpcPacket->TempHead().Add(rpc::Header::from_addr, address);
				asio::post(this->mMainContext, [this, msg = rpcPacket.release()] {
					this->mComponent->OnMessage(msg, nullptr);
				});
			}
		}
	}
}