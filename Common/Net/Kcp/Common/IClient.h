//
// Created by 64658 on 2024/10/26.
//

#pragma once
#include "Rpc/Client/Message.h"
#include "Net/Network/Tcp/Asio.h"
#include "Proto/Message/IProto.h"
#include "Entity/Component/IComponent.h"

using asio_udp = asio::ip::udp;
constexpr int KCP_SHORT_COUNT = std::numeric_limits<unsigned short>::max() - 512;

namespace kcp
{
	class IClient
	{
	public:
		virtual void StartReceive() { };
		virtual void Send(tcp::IProto * message) = 0;
	};
}
