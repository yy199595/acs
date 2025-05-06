//
// Created by 64658 on 2024/10/24.
//

#pragma once
#include "asio.hpp"
#include "Entity/Component/Component.h"
#include "Kcp/Common/KcpServer.h"
#include "Rpc/Common/Message.h"
#include "Server/Component/ITcpComponent.h"
using asio_udp = asio::ip::udp;

namespace acs
{
	class KcpComponent final : public Component, public rpc::IInnerSender, public IRpc<rpc::Message, rpc::Message>
	                           , public IFrameUpdate, public INetListen
	{
	public:
		KcpComponent();
		~KcpComponent() final = default;
	private:
		bool LateAwake() final;
		bool StopListen() final;
		void OnFrameUpdate(long long) noexcept final;
		bool StartListen(const acs::ListenConfig &listen) final;
	private:
		kcp::IClient * GetClient(int id);
		void OnMessage(rpc::Message *request, rpc::Message *response) noexcept final;
	public:
		int Send(int id, rpc::Message *message) noexcept final;
		char GetNet() const noexcept final { return rpc::Net::Kcp; }
	private:
		int OnRequest(rpc::Message * message);
	private:
		ListenConfig mConfig;
		class NodeComponent * mActor;
		asio::streambuf mReceiveBuffer;
		asio_udp::endpoint mRemotePoint;
		class DispatchComponent * mDispatch;
		std::shared_ptr<kcp::Server> mKcpServer;
		std::unordered_map<int, std::shared_ptr<kcp::IClient>> mClients;
	};
}
