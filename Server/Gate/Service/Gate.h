//
// Created by zmhy0073 on 2021/12/1.
//

#ifndef GAMEKEEPER_GATESERVICE_H
#define GAMEKEEPER_GATESERVICE_H
#include"Message/s2s.pb.h"
#include"Rpc/Service/PhysicalRpcService.h"
namespace Sentry
{
	class Gate final : public PhysicalRpcService
	{
	 public:
		Gate();
	 private:
		int Ping(long long userId);
		int Logout(long long userId);
		int Login(const Rpc::Packet & packet);
		int Allocation(long long userId, s2s::allot::response & response);
	private:
		bool Awake() final;
		bool OnInit() final;
		bool OnStart() final;
        void OnClose() final;
		int OnLogin(long long userId);
	 private:
		std::string mInnerAddress;
		std::string mOuterAddress;
		class RpcService * mUserService;
		class NodeMgrComponent * mNodeComponent;
		class OuterNetComponent* mOuterComponent;
		std::unordered_map<std::string, long long> mTokens;
	};

}

#endif //GAMEKEEPER_GATESERVICE_H
