//
// Created by 64658 on 2024/10/26.
//

#ifndef APP_ICLIENT_H
#define APP_ICLIENT_H
#include "Rpc/Client/Message.h"
#include "Net/Network/Tcp/Asio.h"
#include "Proto/Message/IProto.h"
#include "Entity/Component/IComponent.h"

using asio_udp = asio::ip::udp;
constexpr int SHORT_COUNT = std::numeric_limits<unsigned short>::max();

namespace udp
{
	class IClient
	{
	public:
		virtual void StartReceive() = 0;
		virtual bool Send(const std::string & addr, tcp::IProto * message) = 0;
	};
}

#endif //APP_ICLIENT_H
