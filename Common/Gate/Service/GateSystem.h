//
// Created by zmhy0073 on 2021/12/1.
//

#ifndef GAMEKEEPER_GATESERVICE_H
#define GAMEKEEPER_GATESERVICE_H
#include"Message/s2s/s2s.pb.h"
#include"Core/Map/HashMap.h"
#include"Rpc/Service/RpcService.h"
namespace acs
{
	class GateSystem final : public RpcService
	{
	 public:
		GateSystem();
	 private:
		int Ping(long long userId);
		int Login(const rpc::Message & request);
		int Logout(const rpc::Message & request);
	private:
		bool Awake() final;
		bool OnInit() final;
	private:
		bool AllotServer(std::vector<int> & servers);
	 private:
		class ActorComponent * mActorComponent;
	};

}

#endif //GAMEKEEPER_GATESERVICE_H
