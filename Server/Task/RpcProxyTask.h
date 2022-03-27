//
// Created by zmhy0073 on 2021/12/1.
//

#ifndef GAMEKEEPER_RPCPROXYTASK_H
#define GAMEKEEPER_RPCPROXYTASK_H
#include"Async/RpcTask/RpcTaskSource.h"

namespace Sentry
{
	class RpcComponent;
	class GateComponent;
	class RpcProxyTask : public IRpcTask
	{
	 public:
		RpcProxyTask();
		~RpcProxyTask() = default;
	 public:
		void InitProxyTask(long long rpcId, long long sockId,
			GateComponent* component, RpcComponent* rpcComponent);
		long long GetRpcId() final
		{
			return this->mTaskRpcId;
		}
	 protected:
		int GetTimeout() final
		{
			return 0;
		}
		void OnResponse(std::shared_ptr<com::Rpc_Response> response) final;
	 private:
		long long mRpcId;
		long long mSockId;
		long long mTaskRpcId;
		GateComponent* mGateComponent;
	};
}


#endif //GAMEKEEPER_RPCPROXYTASK_H
