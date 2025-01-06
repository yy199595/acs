#pragma once
#include"Core/Map/HashMap.h"
#include"Core/Queue/Queue.h"
#include"Rpc/Common/Message.h"
#include"Core/Set/HashSet.h"
#include"Rpc/Client/InnerTcpClient.h"
#include"Server/Component/ListenerComponent.h"
struct lua_State;
namespace acs
{
	// 管理内网rpc的session
	class InnerTcpComponent : public Component, public ITcpListen,
							  public IRpc<rpc::Message, rpc::Message>, public IServerRecord, public rpc::IInnerSender
	{
	 public:
		InnerTcpComponent();
		~InnerTcpComponent() override = default;
	 public:
		void StartClose(int id) final;
		void OnSendFailure(int id, rpc::Message * message) final;
		void OnMessage(rpc::Message * message, rpc::Message *) noexcept final ;
	private:
		int Send(int id, rpc::Message * message) final;
		char GetNet() const final { return rpc::Net::Tcp; }
	protected:
        bool LateAwake() final;
		bool OnListen(tcp::Socket * socket) final;
		void OnClientError(int id, int code) final;
		void OnRecord(json::w::Document & document) final;
	private:
		int OnRequest(rpc::Message * message) noexcept;
		int OnForward(rpc::Message * message);
		rpc::InnerTcpClient * GetClient(int id);
	private:
		class ActorComponent * mActor;
		math::NumberPool<int> mNumPool;
		class ThreadComponent * mThread;
        class DispatchComponent* mDispatch;
		std::unordered_map<int, std::shared_ptr<rpc::InnerTcpClient>> mClients; //本地客户端(连接别的)
	};
}
