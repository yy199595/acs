//
// Created by 64658 on 2024/10/24.
//

#ifndef APP_UDPCLIENT_H
#define APP_UDPCLIENT_H
#include "IClient.h"

namespace udp
{
	class Client : public IClient
	{
	public:
		typedef acs::IRpc<rpc::Packet, rpc::Packet> Component;
		explicit Client(asio::io_context & io, Component * component, int id);
		~Client() = default;
	public:
		inline int SockId() const { return this->mSockId; }
		bool Send(const std::string & addr, tcp::IProto * message) final;
		inline asio_udp::socket & Socket() { return this->mSocket; }
	public:
		void StartReceive() final;
	private:
		void OnSendMessage();
		void OnSendMessage(const Asio::Code & code);
	private:
		int mSockId;
		Component * mComponent;
		asio_udp::socket mSocket;
		asio::io_context & mContext;
		asio::streambuf mSendBuffer;
		asio::streambuf mRecvBuffer;
		asio_udp::endpoint mLocalEndpoint;
		//char mSendBuffer[std::numeric_limits<unsigned short>::max()] = { 0 };
	};
}


#endif //APP_UDPCLIENT_H
