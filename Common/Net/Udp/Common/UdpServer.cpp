//
// Created by 64658 on 2024/10/26.
//

#include "UdpServer.h"
#include "Util/Tools/String.h"

namespace udp
{
	Server::Server(asio::io_context& io, udp::Server::Component* component, unsigned short port, Asio::Context & main)
			: mContext(io), mSocket(io, asio::ip::udp::endpoint(asio::ip::udp::v4(), port)), mComponent(component), mMainContext(main)
	{

	}

	void Server::StartReceive()
	{
		auto callback = [this](const asio::error_code& code, size_t size) {

			if (code.value() == Asio::OK)
			{
				this->OnReceive(size);
			}
			if (code == asio::error::operation_aborted)
			{
				return;
			}
			asio::post(this->mContext, [this] { this->StartReceive(); });
		};
		this->mSocket.async_receive_from(this->mRecvBuffer.prepare(udp::BUFFER_COUNT),this->mSenderPoint, callback);
	}

	void Server::OnReceive(size_t size)
	{
		this->mRecvBuffer.commit(size);
		std::istream is(&this->mRecvBuffer);
		std::unique_ptr<rpc::Message> rpcPacket = std::make_unique<rpc::Message>();
		{
			tcp::Data::Read(is, rpcPacket->GetProtoHead());
			unsigned short len = rpcPacket->GetProtoHead().Len;
			if ((size - rpc::RPC_PACK_HEAD_LEN) != len)
			{
				return;
			}
			if (rpcPacket->OnRecvMessage(is, len) != tcp::ReadDone)
			{
				return;
			}
			unsigned short port = this->mSenderPoint.port();
			std::string ip = this->mSenderPoint.address().to_string();
			const std::string address = fmt::format("{}:{}", ip, port);
#ifdef __DEBUG__
			rpcPacket->TempHead().Add(rpc::Header::from_addr, address);
#endif
			rpcPacket->SetNet(rpc::Net::Udp);
			rpcPacket->TempHead().Add(rpc::Header::from_addr, address);
			asio::post(this->mMainContext, [this, msg = rpcPacket.release()]{ this->mComponent->OnMessage(msg, nullptr); });
		}
		this->mRecvBuffer.consume(size);
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
			auto callback = [this, length, message](const asio::error_code& code, size_t size) {
				delete message;
				if (code.value() != Asio::OK)
				{
					return;
				}
				this->mSendBuffer.consume(size);
			};
			asio::ip::udp::endpoint endpoint(asio::ip::make_address(ip), port);
			this->mSocket.async_send_to(this->mSendBuffer.data(), endpoint, callback);
		});
		return true;
	}
}