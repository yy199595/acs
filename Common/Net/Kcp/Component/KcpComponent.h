//
// Created by 64658 on 2024/10/24.
//

#pragma once
#include <array>
#include "asio.hpp"
#include "Rpc/Client/Message.h"
#include "Rpc/Interface/ISend.h"
#include "Net/Kcp/Common/KcpServer.h"
#include "Entity/Component/Component.h"
using asio_udp = asio::ip::udp;

namespace acs
{
	class KcpComponent : public Component, public ISender, public IRpc<rpc::Packet, rpc::Packet>
	{
	public:
		KcpComponent();
		~KcpComponent() final = default;
	private:
		bool LateAwake() final;
	private:
		kcp::IClient * GetClient(int id);
		kcp::IClient * GetClient(const std::string & addr);
		kcp::IClient * MakeClient(const std::string & addr);
		kcp::IClient * MakeSession(const std::string & addr);
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
		std::unique_ptr<kcp::Server> mKcpServer;
		std::unordered_map<std::string, std::unique_ptr<kcp::IClient>> mClients;
	};
}
