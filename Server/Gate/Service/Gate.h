//
// Created by zmhy0073 on 2021/12/1.
//

#ifndef GAMEKEEPER_GATESERVICE_H
#define GAMEKEEPER_GATESERVICE_H
#include"Message/s2s/s2s.pb.h"
#include"Rpc/Service/PhysicalRpcService.h"
namespace Tendo
{
	class Gate final : public PhysicalRpcService, public IEvent<DisConnectEvent>
	{
	 public:
		Gate();
	 private:
		int Ping(long long userId);
		int Logout(long long userId);
		int Login(const Msg::Packet & packet);
		int Allocation(long long userId, s2s::allot::response & response);
	private:
		bool Awake() final;
		bool OnInit() final;
		bool OnStart() final;
        void OnClose() final;
		void OnEvent(const DisConnectEvent *message) final;
		int OnLogin(long long userId, const std::string & token);
	 private:
		int mIndex;
		std::string mTable;
		std::string mInnerAddress;
		std::string mOuterAddress;
		class RpcService * mUserService;
		class LocationComponent * mNodeComponent;
		class OuterNetComponent* mOuterComponent;
		std::unordered_map<std::string, long long> mTokens;
	};

}

#endif //GAMEKEEPER_GATESERVICE_H
