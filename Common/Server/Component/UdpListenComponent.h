//
// Created by 64658 on 2024/10/24.
//

#ifndef APP_UDPLISTENCOMPONENT_H
#define APP_UDPLISTENCOMPONENT_H
#include <array>
#include <asio.hpp>
#include "Rpc/Client/Message.h"
#include "Rpc/Interface/ISend.h"
#include "Network/Udp/UdpClient.h"
#include "Entity/Component/Component.h"
using asio_udp = asio::ip::udp;

namespace acs
{
	class UdpListenComponent : public Component, public ISender, public IComplete
	{
	public:
		UdpListenComponent();
		~UdpListenComponent() final = default;
	private:
		bool LateAwake() final;
		void Complete() final;
	private:
		void StartReceive();
		void OnMessage(rpc::Packet * message);
	public:
		int Send(int id, rpc::Packet *message) final;
	private:
		int OnLogin(rpc::Packet * message);
		int OnLogout(rpc::Packet * message);
	private:
		int mPort;
		asio::streambuf mReceiveBuffer;
		asio_udp::endpoint mRemotePoint;
		math::NumberPool<int> mNumberPool;
		class DispatchComponent * mDispatch;
		std::unique_ptr<asio_udp::socket> mSocket;
		std::unordered_map<int, std::unique_ptr<udp::Client>> mClients;
	};
}


#endif //APP_UDPLISTENCOMPONENT_H
