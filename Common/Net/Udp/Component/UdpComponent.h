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
#include "Server/Component/ITcpComponent.h"
using asio_udp = asio::ip::udp;

namespace acs
{
	class UdpComponent : public Component, public ISender, public IRpc<rpc::Message, rpc::Message>,
						 public INetListen
	{
	public:
		UdpComponent();
		~UdpComponent() final = default;
	private:
		bool LateAwake() final;
		bool StopListen() final;
		bool StartListen(const acs::ListenConfig &listen) final;
	private:
		udp::Client * GetClient(int id);
		void OnMessage(rpc::Message *request, rpc::Message *response) final;
	public:
		int Send(int id, rpc::Message *message) final;
	private:
		int OnRequest(rpc::Message * message);
	private:
		class ActorComponent * mActor;
		asio::streambuf mReceiveBuffer;
		asio_udp::endpoint mRemotePoint;
		class DispatchComponent * mDispatch;
		std::unique_ptr<udp::Server> mUdpServer;
		std::unordered_map<int, std::unique_ptr<udp::Client>> mClients;
	};
}
