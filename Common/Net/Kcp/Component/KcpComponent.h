//
// Created by 64658 on 2024/10/24.
//

#pragma once
#include <array>
#include "asio.hpp"
#include "Rpc/Common/Message.h"
#include "Kcp/Common/KcpServer.h"
#include "Entity/Component/Component.h"
#include "Server/Component/ITcpComponent.h"
using asio_udp = asio::ip::udp;

namespace acs
{
	class KcpComponent : public Component, public rpc::IInnerSender, public IRpc<rpc::Message, rpc::Message>
			, public IFrameUpdate, public INetListen
	{
	public:
		KcpComponent();
		~KcpComponent() final = default;
	private:
		bool LateAwake() final;
		bool StopListen() final;
		void OnFrameUpdate(long long) final;
		bool StartListen(const acs::ListenConfig &listen) final;
	private:
		kcp::IClient * GetClient(int id);
		void OnMessage(rpc::Message *request, rpc::Message *response) final;
	public:
		int Send(int id, rpc::Message *message) final;
		char GetNet() const final { return rpc::Net::Kcp; }
	private:
		int OnRequest(rpc::Message * message);
	private:
		ListenConfig mConfig;
		class ActorComponent * mActor;
		asio::streambuf mReceiveBuffer;
		asio_udp::endpoint mRemotePoint;
		class DispatchComponent * mDispatch;
		std::unique_ptr<kcp::Server> mKcpServer;
		std::unordered_map<int, std::unique_ptr<kcp::IClient>> mClients;
	};
}
