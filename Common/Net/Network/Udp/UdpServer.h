//
// Created by 64658 on 2024/10/26.
//

#ifndef APP_UDPSERVER_H
#define APP_UDPSERVER_H

#include "IClient.h"
#include "Core/Map/HashMap.h"
namespace udp
{
	class Server : public IClient
	{
	public:
		typedef acs::IRpc<rpc::Packet, rpc::Packet> Component;
		Server(asio::io_context & io, Component * component, unsigned short port);
	public:
		void StartReceive() final;
		bool Send(const std::string & addr, tcp::IProto * message) final;
	private:
		Component * mComponent;
		asio::io_context & mContext;
		asio::streambuf mRecvBuffer;
		asio::streambuf mSendBuffer;
		asio::ip::udp::socket mSocket;
		asio::ip::udp::endpoint mSenderPoint;
		std::unordered_map<int, asio::ip::udp::endpoint> mClients;
	};
}


#endif //APP_UDPSERVER_H
