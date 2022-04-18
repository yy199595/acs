
//
// Created by mac on 2021/11/28.
//

#ifndef GAMEKEEPER_PROTOPROXYCOMPONENT_H
#define GAMEKEEPER_PROTOPROXYCOMPONENT_H
#include"Component/Component.h"
#include"Async/TaskSource.h"
#include"Task/RequestTaskQueueSource.h"
namespace Sentry
{
	class GateComponent final : public Component,
						  public IClientRpc<c2s::Rpc_Request, c2s::Rpc_Response>
	{
	 public:
		GateComponent() = default;
		~GateComponent() final = default;
	 protected:
		bool LateAwake() final;
	 public:
		void OnConnect(const std::string & address) final;
		XCode OnRequest(std::shared_ptr<c2s::Rpc_Request> request) final;
		XCode OnResponse(const std::string & address, std::shared_ptr<c2s::Rpc_Response> response) final;
	 private:
		void OnNotLogin(std::shared_ptr<c2s::Rpc_Request> request);
	 private:
		class TaskComponent * mTaskComponent;
		class RpcClientComponent * mRpcClientComponent;
		class GateClientComponent* mGateClientComponent;
		std::unordered_map<long long, long long> mClientMap;
	};
}


#endif //GAMEKEEPER_PROTOPROXYCOMPONENT_H
