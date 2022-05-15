
//
// Created by mac on 2021/11/28.
//

#ifndef GAMEKEEPER_PROTOPROXYCOMPONENT_H
#define GAMEKEEPER_PROTOPROXYCOMPONENT_H
#include"Component/Component.h"
#include"Async/TaskSource.h"
#include"Pool/ProtoPool.h"
#include"Task/RequestTaskQueueSource.h"

namespace Sentry
{
	class GateComponent final : public Component,
						  public IClientRpc<c2s::Rpc_Request, c2s::Rpc_Response>
	{
	 public:
		GateComponent() = default;
		~GateComponent() final = default;
	 public:
		XCode OnRequest(std::shared_ptr<c2s::Rpc_Request> request) final;
		XCode OnResponse(const std::string & address, std::shared_ptr<c2s::Rpc_Response> response) final;
	private:
		bool LateAwake() final;
		void OnUserRequest(const RpcInterfaceConfig * config, std::shared_ptr<com::Rpc::Request> request);
		XCode HandlerRequest(const RpcInterfaceConfig * config, std::shared_ptr<com::Rpc::Request> request, std::shared_ptr<c2s::Rpc::Response> response);
	 private:
		class TaskComponent * mTaskComponent;
		class TimerComponent * mTimerComponent;
		class UserSyncComponent * mUserSyncComponent;
		class GateClientComponent* mGateClientComponent;
		ProtoPool<com::Rpc::Request, 100> mRequestPool;
		ProtoPool<com::Rpc::Response, 100> mResponsePool;
		ProtoPool<c2s::Rpc::Response, 100> mCliResponsePool;
	};
}


#endif //GAMEKEEPER_PROTOPROXYCOMPONENT_H
