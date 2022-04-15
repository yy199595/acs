
//
// Created by mac on 2021/11/28.
//

#ifndef GAMEKEEPER_PROTOPROXYCOMPONENT_H
#define GAMEKEEPER_PROTOPROXYCOMPONENT_H
#include"Component/Component.h"
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
		XCode OnRequest(std::shared_ptr<c2s::Rpc_Request> request) final;
		XCode OnResponse(long long sockId, std::shared_ptr<c2s::Rpc_Response> response) final;
	 private:
		std::string mProtoName;
		class RpcComponent* mRpcComponent;
		class GateClientComponent* mGateClientComponent;
	};
}


#endif //GAMEKEEPER_PROTOPROXYCOMPONENT_H
