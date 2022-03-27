#pragma once

#include"Component/Component.h"
#include"Script/LuaTable.h"
#include"Other/MultiThreadQueue.h"
#include"Network/Rpc/ProtoRpcClient.h"
#include"Pool/AllotorPool.h"
namespace Sentry
{
	// 管理所有session
	class RpcClientComponent : public Component, public ISocketListen,
							   public IRpc<com::Rpc_Request, com::Rpc_Response>
	{
	 public:
		RpcClientComponent() = default;
		~RpcClientComponent() override = default;
	 public:
		void StartClose(long long id) final;
		void OnConnectAfter(long long id, XCode code) final;
		void OnCloseSocket(long long socketId, XCode code) final;
		void OnRequest(std::shared_ptr<com::Rpc_Request> request) final;
		void OnResponse(std::shared_ptr<com::Rpc_Response> response) final;

	 protected:
		void OnListen(std::shared_ptr<SocketProxy> socket) final;
	 public:
		std::shared_ptr<ProtoRpcClient> GetSession(long long id);
		std::shared_ptr<ProtoRpcClient> GetSession(const std::string& address);
		std::shared_ptr<ProtoRpcClient> MakeSession(const std::string& name, const std::string& address);
	 protected:
		bool Awake() final;
		bool LateAwake() final;
	 public:
		bool CloseSession(long long id);
		bool Send(long long id, std::shared_ptr<com::Rpc_Request> message);
		bool Send(long long id, std::shared_ptr<com::Rpc_Response> message);
	 private:
		class RpcComponent* mRpcComponent;
		class ThreadPoolComponent* mTaskComponent;
		std::unordered_map<std::string, long long> mAddressClientMap;
		std::unordered_map<long long, std::shared_ptr<ProtoRpcClient>> mRpcClientMap;
	};
}
