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
		typedef acs::IRpc<rpc::Message, rpc::Message> Component;
		Server(asio::io_context & io, Component * component, unsigned short port, Asio::Context & main);
	public:
		void Start();
		void RemoveSession(const std::string & address);
		bool Send(const std::string & addr, tcp::IProto * message);
		inline asio::ip::udp::socket & Socket() { return this->mSocket; }
	private:
		void Update();
		void StartTimer();
		void StartReceive();
		void OnReceive(size_t size);
		kcp::Session * GetSession(const std::string & address);
	private:
		Component * mComponent;
		asio::system_timer mTimer;
		asio::io_context & mContext;
		asio::streambuf mSendBuffer;
		asio::ip::udp::socket mSocket;
		Asio::Context & mMainContext;
		std::chrono::milliseconds mTime;
		asio::ip::udp::endpoint mSenderPoint;
		std::array<char, kcp::BUFFER_COUNT> mRecvBuffer;
		std::array<char, kcp::BUFFER_COUNT> mDecodeBuffer;
		std::unordered_map<std::string, std::unique_ptr<kcp::Session>> mClients;
	};
}
