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
	public:
		void Send(tcp::IProto *message) final;
	private:
		asio_udp::socket & mSocket;
		asio_udp::endpoint mRemote;
		asio::streambuf mSendBuffer;
	};
}
