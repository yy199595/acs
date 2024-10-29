//
// Created by 64658 on 2024/10/26.
//

#pragma once

#include "IClient.h"
#include "KcpSession.h"
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
		bool Send(const std::string & addr, tcp::IProto * message);
		inline asio::ip::udp::socket & Socket() { return this->mSocket; }
	private:
		void Update();
		void OnReceive(size_t size);
		void OnUpdate(const asio::error_code & code);
		kcp::Session * GetSession(const std::string & address);
	private:
		Component * mComponent;
		asio::system_timer mTimer;
		asio::io_context & mContext;
		asio::streambuf mSendBuffer;
		asio::ip::udp::socket mSocket;
		asio::ip::udp::endpoint mSenderPoint;
		std::array<char, KCP_BUFFER_SIZE> mRecvBuffer;
		std::array<char, KCP_BUFFER_SIZE> mDecodeBuffer;
		std::unordered_map<std::string, std::unique_ptr<kcp::Session>> mClients;
	};
}
