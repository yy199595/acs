//
// Created by 64658 on 2024/10/24.
//

#pragma once
#include "IClient.h"

namespace kcp
{
	class Client : public IClient
	{
	public:
		typedef acs::IRpc<rpc::Packet, rpc::Packet> Component;
		explicit Client(asio::io_context & io, Component * component, asio_udp::endpoint & remote);
		~Client(){ ikcp_release(this->mKcp); }
	public:
		void Send(tcp::IProto * message) final;
		void Send(const char *buf, int len) final;
		inline asio_udp::socket & Socket() { return this->mSocket; }
	public:
		void StartReceive() final;
		void Update(long long ms) final;
	private:
		void OnReceive(const std::string & addr, const char * buf, int size);
	private:
		ikcpcb * mKcp;
		std::ostream mSendStream;
		Component * mComponent;
		asio_udp::socket mSocket;
		asio::system_timer mTimer;
		asio::io_context & mContext;
		asio::streambuf mSendBuffer;
		asio::streambuf mReceiveBuffer;
		asio_udp::endpoint mRemoteEndpoint;
		asio_udp::endpoint mLocalEndpoint;
		std::array<char, kcp::BUFFER_COUNT> mDecodeBuffer;
		//char mSendBuffer[std::numeric_limits<unsigned short>::max()] = { 0 };
	};
}
