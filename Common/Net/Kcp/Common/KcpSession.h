//
// Created by 64658 on 2024/10/28.
//

#pragma once

#include "IClient.h"
namespace kcp
{
	class Session : public IClient
	{
	public:
		Session(asio_udp::socket & sock, asio_udp::endpoint & endpoint);
		~Session() { ikcp_release(this->mKcp);}
	public:
		void Update(long long ms) final;
		void Send(tcp::IProto *message) final;
		void Send(const char *buf, int len) final;
		inline long long GetLastTime() const { return this->mLastTime; }
		int Decode(const char * message, int len, char * buffer);
		inline const std::string & GetAddress() const { return this->mAddress;}
	private:
		ikcpcb * mKcp;
		long long mLastTime;
		std::string mAddress;
		std::ostream mSendStream;
		asio_udp::socket & mSocket;
		asio_udp::endpoint mRemote;
		asio::streambuf mSendBuffer;
	};
}
