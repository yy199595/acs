//
// Created by 64658 on 2024/10/26.
//

#pragma once
#include <array>
#include "Kcp/Src/ikcp.h"
#include "Rpc/Client/Message.h"
#include "Net/Network/Tcp/Asio.h"
#include "Proto/Message/IProto.h"
#include "Entity/Component/IComponent.h"

using asio_udp = asio::ip::udp;
constexpr int KCP_TIME_OUT = 20; //xx秒
constexpr int KCP_BUFFER_SIZE = 1024 * 10;

namespace kcp
{
	class IClient
	{
	public:
		virtual void StartReceive() { };
		virtual void Update(long long t) = 0;
		virtual void Send(tcp::IProto * message) { };
		virtual void Send(const char * buf, int len) = 0;
	};

	inline int OnKcpSend(const char *buf, int len, struct IKCPCB *kcp, void *user)
	{
		IClient * client = (IClient*)user;
		if(client != nullptr)
		{
			client->Send(buf, len);
		}
		return 0;
	}
}
