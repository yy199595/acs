//
// Created by 64658 on 2024/10/26.
//

#pragma once
#include "IClient.h"
#include "Core/Map/HashMap.h"
namespace udp
{
	class Server
	{
	public:
		typedef acs::IRpc<rpc::Packet, rpc::Packet> Component;
		Server(asio::io_context & io, Component * component, unsigned short port);
	public:
		void StartReceive();
		inline asio::ip::udp::socket & Socket() { return this->mSocket; }
	public:
		bool Send(const std::string & addr, tcp::IProto * message);
		bool Send(const std::string & ip, unsigned short port, tcp::IProto * message);
	private:
		void OnReceive(size_t size);
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
