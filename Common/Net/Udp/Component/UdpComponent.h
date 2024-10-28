//
// Created by 64658 on 2024/10/24.
//

#pragma once;
#include <array>
#include "asio.hpp"
#include "Rpc/Client/Message.h"
#include "Rpc/Interface/ISend.h"
#include "Udp/Common/UdpServer.h"
#include "Udp/Common/UdpClient.h"
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
		udp::Client * GetClient(int id);
		void OnMessage(rpc::Packet *request, rpc::Packet *response) final;
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
