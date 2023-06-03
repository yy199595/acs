//
// Created by zmhy0073 on 2021/12/1.
//

#ifndef GAMEKEEPER_GATESERVICE_H
#define GAMEKEEPER_GATESERVICE_H
#include"Message/s2s/s2s.pb.h"
#include"Rpc/Service/RpcService.h"
namespace Tendo
{
	class Gate final : public RpcService
	{
	 public:
		Gate();
	 private:
		int Ping(long long userId);
		int Logout(long long userId);
		int Login(const Msg::Packet & packet);
		int Allocation(const s2s::allot::request & request);
	private:
		bool Awake() final;
		bool OnInit() final;
        void OnStop() final;
		int OnLogin(long long userId, const std::string & token);
	private:
		std::shared_ptr<class PlayerActor> NewPlayer(long long userId);
	 private:
		int mIndex;
		std::string mInnerAddress;
		std::string mOuterAddress;
		class OuterNetComponent* mOuterComponent;
		class ActorMgrComponent * mActorComponent;
		std::unordered_map<std::string, long long> mTokens;
	};

}

#endif //GAMEKEEPER_GATESERVICE_H
