//
// Created by mac on 2021/11/28.
//

#ifndef GAMEKEEPER_GATECLIENTCOMPONENT_H
#define GAMEKEEPER_GATECLIENTCOMPONENT_H

#include"Core/Map/HashMap.h"
#include"Core/Queue/Queue.h"
#include"Log/Common/Logger.h"
#include"Core/Pool/ArrayPool.h"
#include"Gate/Client/OuterClient.h"
#include"Rpc/Interface/ISend.h"
#include"Server/Component/ListenerComponent.h"

namespace rpc
{
    class Packet;
	class InnerClient;
}

namespace acs
{
	class OuterNetComponent final : public Component, public ITcpListen,
									public IRpc<rpc::Packet, rpc::Packet>, public IServerRecord
	{
	 public:
		OuterNetComponent();
		~OuterNetComponent() final = default;
	public:
		void Broadcast(rpc::Packet * message);
		void StartClose(int id, int code) final;
	public:
		bool SendBySockId(int id, rpc::Packet * message);
	private:
		bool LateAwake() final;
		int OnRequest(rpc::Packet * message);
	private:
		void OnClientError(int id, int code) final;
		void OnSendFailure(int id, rpc::Packet *message) final;
		void OnMessage(rpc::Packet * request, rpc::Packet *) final;
	private:
		bool OnListen(tcp::Socket * socket) final;
		void OnRecord(json::w::Document & document) final;
	private:
		void OnPlayerLogin(long long userId, int sockId);
		void OnPlayerLogout(long long userId, int sockId);
	private:
		int mWaitCount;
		int mMaxConnectCount;
		math::NumberPool<int> mNumPool;
		class RouterComponent * mRouter;
		math::NumberPool<int> mSocketPool;
		class ActorComponent * mActComponent;
		custom::Queue<rpc::Packet *> mBroadCastMessages; //广播消息
		std::unordered_map<int, std::shared_ptr<rpc::OuterClient>> mGateClientMap;
		//std::unordered_map<int, std::unique_ptr<rpc::InnerClient>> mForwardClientMap;
	};
}


#endif //GAMEKEEPER_GATECLIENTCOMPONENT_H
