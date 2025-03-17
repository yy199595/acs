//
// Created by 64658 on 2024/10/24.
//

#include "UdpClient.h"
#include "Util/Tools/String.h"
#include "Entity/Actor/App.h"
#include "Log/Common/CommonLogDef.h"

namespace udp
{
	Client::Client(asio::io_context& io, udp::Client::Component* component, asio_udp::endpoint & remote, Asio::Context & main)
			: mContext(io), mComponent(component), mSocket(io, asio_udp::endpoint(asio_udp::v4(), 0)), mRemoteEndpoint(remote), mMainContext(main)
	{

	}

	void Client::Send(tcp::IProto* message)
	{
		std::shared_ptr<Client> self = this->shared_from_this();
		asio::post(this->mContext, [self, message]()
		{
			std::ostream stream(&self->mSendBuffer);
			int length = message->OnSendMessage(stream);
			auto callback = [self, length, message](const asio::error_code& code, size_t size)
			{
				delete message;
				if (code.value() == Asio::OK)
				{
					self->mSendBuffer.consume(size);
				}
			};
			self->mSocket.async_send_to(self->mSendBuffer.data(), self->mRemoteEndpoint, callback);
		});
	}

	void Client::StartReceive()
	{
		std::shared_ptr<Client> self = this->shared_from_this();
		auto callback = [self](const asio::error_code& code, size_t size) {
			if (code.value() == Asio::OK)
			{
				self->mRecvBuffer.commit(size);
				unsigned short port = self->mLocalEndpoint.port();
				std::string ip = self->mLocalEndpoint.address().to_string();
				std::string address = fmt::format("{}:{}", ip, port);

				self->OnReceive(address, size);
				self->mRecvBuffer.consume(size);
			}
			asio::post(self->mSocket.get_executor(), [self] { self->StartReceive(); });
		};
		this->mSocket.async_receive_from(this->mRecvBuffer.prepare(udp::BUFFER_COUNT),
				this->mLocalEndpoint, callback);
	}

	void Client::OnReceive(const std::string & address, size_t size)
	{
		std::istream is(&this->mRecvBuffer);
		std::unique_ptr<rpc::Message> rpcPacket = std::make_unique<rpc::Message>();
		{
			tcp::Data::Read(is, rpcPacket->GetProtoHead());
			if((size - rpc::RPC_PACK_HEAD_LEN) == rpcPacket->GetProtoHead().Len)
			{
				if (rpcPacket->OnRecvMessage(is, rpcPacket->GetProtoHead().Len) == tcp::read::done)
				{
					rpcPacket->SetNet(rpc::Net::Udp);
					std::shared_ptr<Client> self = this->shared_from_this();
					rpcPacket->TempHead().Add(rpc::Header::from_addr, address);
					asio::post(this->mMainContext, [self, msg = rpcPacket.release()] { self->mComponent->OnMessage(msg, msg); });
				}
			}
		}
	}
}