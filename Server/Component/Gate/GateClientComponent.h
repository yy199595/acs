//
// Created by mac on 2021/11/28.
//

#ifndef GAMEKEEPER_GATECLIENTCOMPONENT_H
#define GAMEKEEPER_GATECLIENTCOMPONENT_H
#include"Component/Component.h"
namespace Sentry
{
	class GateClientContext;
	class GateClientComponent : public Component, public ISocketListen,
								public IRpc<c2s::Rpc_Request, c2s::Rpc_Response>
	{
	 public:
		GateClientComponent() = default;
		~GateClientComponent() final = default;
	 public:
		void StartClose(const std::string & address) final;
		void OnCloseSocket(const std::string & address, XCode code) final;
		void OnRequest(std::shared_ptr<c2s::Rpc_Request> request) final;
		void OnResponse(std::shared_ptr<c2s::Rpc_Response> response) final {}
	 public:
		bool AddNewUser(const std::string & address, long long userId);
		bool GetUserId(const std::string & address, long long & userId);
		bool GetUserAddress(long long userId, std::string & address);
		std::shared_ptr<GateClientContext> GetGateClient(const std::string & address);
	 public:
		void SendToAllClient(std::shared_ptr<c2s::Rpc::Call> message);
		bool SendToClient(const std::string & address, std::shared_ptr<c2s::Rpc::Call> message);
		bool SendToClient(const std::string & address, std::shared_ptr<c2s::Rpc_Response> message);
	 public:
		void Awake() final;
		bool LateAwake() final;
		bool OnListen(std::shared_ptr<SocketProxy> socket) final;
	 private:
		void CheckPlayerLogout(const std::string & address);
	 private:
		class GateComponent* mGateComponent;
		class TimerComponent* mTimerComponent;
        class TcpServerComponent * mTcpComponent;
		std::unordered_map<std::string, long long> mUserAddressMap;
        std::queue<std::shared_ptr<GateClientContext>> mClientPools;
        std::unordered_map<long long, std::string> mClientAddressMap;
		std::unordered_map<std::string, std::shared_ptr<GateClientContext>> mGateClientMap;
	};
}


#endif //GAMEKEEPER_GATECLIENTCOMPONENT_H
