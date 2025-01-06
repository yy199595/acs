//
// Created by 64658 on 2024/10/26.
//

#ifndef APP_ICLIENT_H
#define APP_ICLIENT_H
#include "Rpc/Common/Message.h"
#include "Net/Network/Tcp/Asio.h"
#include "Proto/Message/IProto.h"
#include "Entity/Component/IComponent.h"

using asio_udp = asio::ip::udp;


namespace udp
{
	constexpr int BUFFER_COUNT = 1200;
	class IClient
	{
	public:
		virtual void StartReceive() { };
		virtual void Send(tcp::IProto * message) = 0;
	};
}

#endif //APP_ICLIENT_H
