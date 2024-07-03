#pragma once
#include"Core/Map/HashMap.h"
#include"Core/Queue/Queue.h"
#include"Rpc/Client/Message.h"
#include"Rpc/Interface/ISend.h"
#include"Core/Set/HashSet.h"
#include"Rpc/Client/InnerClient.h"
#include"Server/Component/ListenerComponent.h"
struct lua_State;
namespace joke
{
	// 管理内网rpc的session
	class InnerNetComponent : public Component, public ITcpListen,
			public IRpc<rpc::Packet, rpc::Packet>, public IServerRecord, public ISender
	{
	 public:
		InnerNetComponent();
		~InnerNetComponent() override = default;
	 public:
		void StartClose(int id) final;
		void OnCloseSocket(int id, int code) final;
		int Send(int id, rpc::Packet * message) final;
		void OnSendFailure(int id, rpc::Packet * message) final;
		void OnMessage(rpc::Packet * message, rpc::Packet *) final;
	protected:
        bool LateAwake() final;
		bool OnListen(tcp::Socket * socket) final;
		void OnRecord(json::w::Document & document) final;
	private:
		void OnRequest(rpc::Packet * message);
		void OnForward(rpc::Packet * message);
		bool GetClient(int id, rpc::InnerClient *& tcpClient);
	private:
		math::NumberPool<int, 100> mNumPool;
		class ActorComponent * mActComponent;
		class ThreadComponent * mNetComponent;
        class DispatchComponent* mDisComponent;
		custom::HashMap<int, rpc::InnerClient *> mClients; //本地客户端(连接别的)
	};
}
