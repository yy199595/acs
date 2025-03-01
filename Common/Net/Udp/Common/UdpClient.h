//
// Created by 64658 on 2024/10/24.
//

#pragma once
#include "IClient.h"

namespace udp
{
	class Client : public IClient, public std::enable_shared_from_this<Client>
	{
	public:
		typedef acs::IRpc<rpc::Message, rpc::Message> Component;
		explicit Client(asio::io_context & io, Component * component, asio_udp::endpoint & remote, Asio::Context &);
		~Client() = default;
	public:
		void Send(tcp::IProto * message) final;
		inline asio_udp::socket & Socket() { return this->mSocket; }
	public:
		void StartReceive() final;
		void OnReceive(const std::string & address, size_t size);
	private:
		Component * mComponent;
		Asio::Context & mMainContext;
		asio_udp::socket mSocket;
		asio::io_context & mContext;
		asio::streambuf mSendBuffer;
		asio::streambuf mRecvBuffer;
		asio_udp::endpoint mRemoteEndpoint;
		asio_udp::endpoint mLocalEndpoint;
		//char mSendBuffer[std::numeric_limits<unsigned short>::max()] = { 0 };
	};
}