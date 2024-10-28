//
// Created by 64658 on 2024/10/26.
//

#pragma once

#include "IClient.h"
#include "Core/Map/HashMap.h"
namespace kcp
{
	class Server
	{
	public:
		typedef acs::IRpc<rpc::Packet, rpc::Packet> Component;
		Server(asio::io_context & io, Component * component, unsigned short port);
	public:
		void StartReceive();
		inline asio::ip::udp::socket & Socket() { return this->mSocket; }
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
