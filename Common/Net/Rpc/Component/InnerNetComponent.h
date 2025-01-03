#pragma once
#include"Core/Map/HashMap.h"
#include"Core/Queue/Queue.h"
#include"Rpc/Client/Message.h"
#include"Rpc/Interface/ISend.h"
#include"Core/Set/HashSet.h"
#include"Rpc/Client/InnerClient.h"
#include"Server/Component/ListenerComponent.h"
struct lua_State;
namespace acs
{
	// 管理内网rpc的session
	class InnerNetComponent : public Component, public ITcpListen,
							  public IRpc<rpc::Message, rpc::Message>, public IServerRecord, public ISender
	{
	 public:
		InnerNetComponent();
		~InnerNetComponent() override = default;
	 public:
		void StartClose(int id) final;
		int Send(int id, rpc::Message * message) final;
		void OnSendFailure(int id, rpc::Message * message) final;
		void OnMessage(rpc::Message * message, rpc::Message *) noexcept final ;
	protected:
        bool LateAwake() final;
		bool OnListen(tcp::Socket * socket) final;
		void OnClientError(int id, int code) final;
		void OnRecord(json::w::Document & document) final;
	private:
		int OnRequest(rpc::Message * message) noexcept;
		int OnForward(rpc::Message * message);
		rpc::InnerClient * GetClient(int id);
	private:
		class ActorComponent * mActor;
		math::NumberPool<int> mNumPool;
		class ThreadComponent * mThread;
        class DispatchComponent* mDispatch;
		std::unordered_map<int, std::shared_ptr<rpc::InnerClient>> mClients; //本地客户端(连接别的)
	};
}
