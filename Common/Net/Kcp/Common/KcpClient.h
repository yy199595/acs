//
// Created by 64658 on 2024/10/24.
//

#pragma once
#include "IClient.h"

namespace kcp
{
	class Client : public IClient, public std::enable_shared_from_this<Client>
	{
	public:
		typedef acs::IRpc<rpc::Message, rpc::Message> Component;
		explicit Client(asio::io_context & io, Component * component, asio_udp::endpoint & remote, Asio::Context & main);
		~Client(){ ikcp_release(this->mKcp); }
	public:
		void Send(const char *buf, int len) final;
		void Send(std::unique_ptr<rpc::Message> & message) final;
		inline asio_udp::socket & Socket() { return this->mSocket; }
	public:
		void StartReceive() final;
		void Update(long long ms) final;
	private:
		void OnReceive(const std::string & addr, const std::string & buf, size_t size);
	private:
		ikcpcb * mKcp;
		std::ostream mSendStream;
		Component * mComponent;
		asio_udp::socket mSocket;
		asio::system_timer mTimer;
		std::string mDecodeBuffer;
		asio::io_context & mContext;
		asio::streambuf mSendBuffer;
		asio::streambuf mReceiveBuffer;
		Asio::Context & mMainContext;
		asio_udp::endpoint mRemoteEndpoint;
		asio_udp::endpoint mLocalEndpoint;
		//char mSendBuffer[std::numeric_limits<unsigned short>::max()] = { 0 };
	};
}
