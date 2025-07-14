//
// Created by 64658 on 2024/10/24.
//

#pragma once;
#include "asio.hpp"
#include "Rpc/Common/Message.h"
#include "Entity/Component/Component.h"
#include "Server/Component/ITcpComponent.h"

namespace udp
{
	using Socket = asio::ip::udp::socket;
	using EndPoint = asio::ip::udp::endpoint;

	constexpr unsigned int REMOVE_TIMEOUT = 20; //超时移除时间
	constexpr unsigned int BUFFER_COUNT = 1200;

	struct ClientInfo
	{
		EndPoint endpoint;
		long long lastTime = 0;
	};
}

namespace acs
{
	class UdpComponent final : public Component, public rpc::IInnerSender,
					public IRpc<rpc::Message, rpc::Message>, public INetListen
	{
	public:
		UdpComponent();
		~UdpComponent() final = default;
	private:
		bool LateAwake() final;
		bool StopListen() final;
		bool StartListen(const acs::ListenConfig &listen) final;
	private:
		void OnMessage(rpc::Message *request, rpc::Message *response) noexcept final;
	public:
		char GetNet() const noexcept final { return rpc::net::udp; }
		int Send(int id, std::unique_ptr<rpc::Message> &message) noexcept final;
		int Send(const std::string & address, std::unique_ptr<rpc::Message> &message);
	private:
		void StartReceive();
		void OnReceiveMessage(size_t size);
		int OnRequest(std::unique_ptr<rpc::Message>& message) noexcept;
	private:
		char mMsg;
		std::string mAddrName;
		std::istream mReadStream;
		std::ostream mSendStream;
		udp::EndPoint mRemotePoint;
		class NodeComponent * mActor;
		asio::streambuf mSendBuffer;
		asio::streambuf mReceiveBuffer;
		class DispatchComponent * mDispatch;
		std::unique_ptr<udp::Socket> mSocket;
	};
}
