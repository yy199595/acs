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
		void StartClose(const std::string & address) final;
		void OnCloseSocket(const std::string & address, XCode code) final;
		void OnConnectAfter(const std::string & address, XCode code) final {}
		void OnRequest(std::shared_ptr<c2s::Rpc_Request> request) final;
		void OnResponse(std::shared_ptr<c2s::Rpc_Response> response) final {}
	 public:
		long long GetUserId(const std::string & address);
		bool AddNewUser(const std::string & address, long long userId);
		std::shared_ptr<RpcGateClient> GetGateClient(const std::string & address);
	 public:
		void SendToAllClient(std::shared_ptr<c2s::Rpc::Call> message);
		bool SendToClient(const std::string & address, std::shared_ptr<c2s::Rpc::Call> message);
		bool SendToClient(const std::string & address, std::shared_ptr<c2s::Rpc_Response> message);
	 public:
		void Awake() final;
		bool LateAwake() final;
		void OnListen(std::shared_ptr<SocketProxy> socket) final;
	 private:
		void CheckPlayerLogout(const std::string & address);
	 private:
		std::set<std::string> mBlackList;
		class RpcComponent* mRpcComponent;
		class GateComponent* mGateComponent;
		class TimerComponent* mTimerComponent;
		std::unordered_map<std::string, long long> mUserAddressMap;
		std::unordered_map<std::string, std::shared_ptr<RpcGateClient>> mGateClientMap;
	};
}


#endif //GAMEKEEPER_GATECLIENTCOMPONENT_H
