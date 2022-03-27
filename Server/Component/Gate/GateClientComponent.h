//
// Created by mac on 2021/11/28.
//

#ifndef GAMEKEEPER_GATECLIENTCOMPONENT_H
#define GAMEKEEPER_GATECLIENTCOMPONENT_H
#include"Component/Component.h"
namespace Sentry
{
	class RpcGateClient;
	class GateClientComponent : public Component, public ISocketListen,
								public IRpc<c2s::Rpc_Request, c2s::Rpc_Response>
	{
	 public:
		GateClientComponent() = default;
		~GateClientComponent() final = default;
	 public:
		void StartClose(long long id) final;
		void OnCloseSocket(long long id, XCode code) final;
		void OnConnectAfter(long long id, XCode code) final
		{
		}
		void OnRequest(std::shared_ptr<c2s::Rpc_Request> request) final;
		void OnResponse(std::shared_ptr<c2s::Rpc_Response> response) final
		{
		}
	 public:
		std::shared_ptr<RpcGateClient> GetGateClient(long long sockId);
		bool SendToClient(long long sockId, std::shared_ptr<c2s::Rpc_Response> message);
	 public:
		bool Awake() final;
		bool LateAwake() final;
		void OnListen(std::shared_ptr<SocketProxy> socket) final;
	 private:
		void CheckPlayerLogout(long long sockId);
	 private:
		std::set<std::string> mBlackList;
		class RpcComponent* mRpcComponent;
		class GateComponent* mGateComponent;
		class TimerComponent* mTimerComponent;
		std::unordered_map<long long, std::shared_ptr<RpcGateClient>> mGateClientMap;
	};
}


#endif //GAMEKEEPER_GATECLIENTCOMPONENT_H
