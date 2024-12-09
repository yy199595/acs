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
	class OuterNetComponent final : public Component, public ITcpListen, public ISecondUpdate,
									public IRpc<rpc::Packet, rpc::Packet>, public IServerRecord
	{
	 public:
		OuterNetComponent();
		~OuterNetComponent() final = default;
	 public:
		void OnTimeout(int id) final;
		void StartClose(int id, int code) final;
		void OnCloseSocket(int id, int code) final;
		void OnSendFailure(int id, rpc::Packet *message) final;
	public:
		bool StopClient(long long userId);
		void Broadcast(rpc::Packet * message);
		bool AddPlayer(long long userId, int sockId);
	public:
		bool SendBySockId(int id, rpc::Packet * message);
		bool SendByPlayerId(long long playerId, rpc::Packet * message);
	private:
		bool LateAwake() final;
		void OnSecondUpdate(int tick) final;
		int OnRequest(long long playerId, rpc::Packet * message);
		void OnMessage(rpc::Packet * request, rpc::Packet *) final;
	private:
		bool OnListen(tcp::Socket * socket) final;
		void OnRecord(json::w::Document & document) final;
	private:
		int mWaitCount;
		int mMaxConnectCount;
		math::NumberPool<int> mNumPool;
		class RouterComponent * mRouter;
		math::NumberPool<int> mSocketPool;
		class ActorComponent * mActComponent;
		custom::HashMap<int, long long> mAddressUserMap;  //fd和玩家id的映射表
		custom::HashMap<long long, int> mUserAddressMap; //验证过的客户端
		custom::Queue<rpc::Packet *> mBroadCastMessages; //广播消息
		std::queue<std::unique_ptr<rpc::OuterClient>> mRemoveClients;
		std::unordered_map<int, std::unique_ptr<rpc::OuterClient>> mGateClientMap;
		//std::unordered_map<int, std::unique_ptr<rpc::InnerClient>> mForwardClientMap;
	};
}


#endif //GAMEKEEPER_GATECLIENTCOMPONENT_H
