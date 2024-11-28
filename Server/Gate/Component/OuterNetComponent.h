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
		void OnTimeout(int id) final;
		void StartClose(int id, int code) final;
		void OnCloseSocket(int id, int code) final;
		void OnSendFailure(int id, rpc::Packet *message) final;
	public:
		bool StopClient(long long userId);
		void Broadcast(rpc::Packet * message);
		bool Send(int id, rpc::Packet * message);
		bool AddPlayer(long long userId, int sockId);
		bool Send(int id, int code, rpc::Packet * message);
		bool SendToPlayer(long long userId, rpc::Packet * message);
		inline size_t GetPlayerCount() const { return this->mUserAddressMap.Size(); }
	private:
		bool LateAwake() final;
		int Forward(int id, rpc::Packet * message);
		int OnRequest(long long playerId, rpc::Packet * message);
		void OnMessage(rpc::Packet * request, rpc::Packet *) final;
	private:
		bool OnListen(tcp::Socket * socket) final;
		void OnRecord(json::w::Document & document) final;
	private:
		int mWaitCount;
		int mMaxConnectCount;
		math::NumberPool<int> mNumPool;
		math::NumberPool<int> mSocketPool;
		class ActorComponent * mActComponent;
		class ThreadComponent * mThreadComponent;
		custom::HashMap<int, long long> mAddressUserMap;  //fd和玩家id的映射表
		custom::HashMap<long long, int> mUserAddressMap; //验证过的客户端
		custom::Queue<rpc::Packet *> mBroadCastMessages; //广播消息
		custom::HashMap<int, rpc::InnerClient *> mForwardClientMap; //中转到内网的客户端
		std::unordered_map<int, std::unique_ptr<rpc::OuterClient>> mGateClientMap;
	};
}


#endif //GAMEKEEPER_GATECLIENTCOMPONENT_H
