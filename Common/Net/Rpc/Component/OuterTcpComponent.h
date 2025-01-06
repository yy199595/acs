//
// Created by mac on 2021/11/28.
//

#ifndef GAMEKEEPER_GATECLIENTCOMPONENT_H
#define GAMEKEEPER_GATECLIENTCOMPONENT_H

#include "Core/Map/HashMap.h"
#include "Core/Queue/Queue.h"
#include "Log/Common/Logger.h"
#include "Core/Pool/ArrayPool.h"
#include "Rpc//Client/OuterTcpClient.h"
#include "Entity/Component/Component.h"
#include "Server/Component/ITcpComponent.h"

namespace rpc
{
    class Message;
	class InnerTcpClient;
}

namespace acs
{
	class OuterTcpComponent final : public Component, public ITcpListen, public rpc::IOuterSender,
									public IRpc<rpc::Message, rpc::Message>, public IServerRecord
	{
	 public:
		OuterTcpComponent();
		~OuterTcpComponent() final = default;
	public:
		void StartClose(int id, int code) final;
	private:
		bool LateAwake() final;
		void Broadcast(rpc::Message * message) final;
		int Send(int id, rpc::Message * message) final;
		inline char GetNet() const final { return rpc::Net::Tcp; }
	private:
		void OnClientError(int id, int code) final;
		void OnSendFailure(int id, rpc::Message *message) final;
		void OnMessage(rpc::Message * request, rpc::Message *) final;
	private:
		bool OnListen(tcp::Socket * socket) final;
		void OnRecord(json::w::Document & document) final;
	private:
		void OnPlayerLogin(long long userId, int sockId);
		void OnPlayerLogout(long long userId, int sockId);
	private:
		int mWaitCount;
		int mMaxConnectCount;
		class GateComponent * mGate;
		math::NumberPool<int> mSocketPool;
		custom::Queue<rpc::Message *> mBroadCastMessages; //广播消息
		std::unordered_map<int, std::shared_ptr<rpc::OuterTcpClient>> mGateClientMap;
		//std::unordered_map<int, std::unique_ptr<rpc::InnerClient>> mForwardClientMap;
	};
}


#endif //GAMEKEEPER_GATECLIENTCOMPONENT_H
