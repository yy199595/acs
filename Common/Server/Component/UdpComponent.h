//
// Created by 64658 on 2024/10/24.
//

#ifndef APP_UDPCOMPONENT_H
#define APP_UDPCOMPONENT_H
#include <array>
#include <asio.hpp>
#include "Rpc/Client/Message.h"
#include "Rpc/Interface/ISend.h"
#include "Network/Udp/UdpClient.h"
#include "Network/Udp/UdpServer.h"
#include "Entity/Component/Component.h"
using asio_udp = asio::ip::udp;

namespace acs
{
	class UdpComponent : public Component, public ISender, public IRpc<rpc::Packet, rpc::Packet>
	{
	public:
		UdpComponent();
		~UdpComponent() final = default;
	private:
		bool LateAwake() final;
	private:
		udp::Client * GetClient(int id, std::string & address);
		void OnMessage(int, rpc::Packet *request, rpc::Packet *response) final;
	public:
		int Send(int id, rpc::Packet *message) final;
	private:
		int OnRequest(rpc::Packet * message);
	private:
		int mPort;
		class ActorComponent * mActor;
		class ThreadComponent * mThread;
		asio::streambuf mReceiveBuffer;
		asio_udp::endpoint mRemotePoint;
		class DispatchComponent * mDispatch;
		std::unique_ptr<udp::Server> mUdpServer;
		std::unordered_map<int, std::unique_ptr<udp::Client>> mClients;
	};
}


#endif //APP_UDPCOMPONENT_H
