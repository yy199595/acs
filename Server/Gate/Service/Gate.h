//
// Created by zmhy0073 on 2021/12/1.
//

#ifndef GAMEKEEPER_GATESERVICE_H
#define GAMEKEEPER_GATESERVICE_H
#include"Message/s2s/s2s.pb.h"
#include"Core/Map/HashMap.h"
#include"Rpc/Service/RpcService.h"
namespace joke
{
	class Gate final : public RpcService
	{
	 public:
		Gate();
	 private:
		int Ping(long long userId);
		int Logout(long long userId);
		int Login(const rpc::Packet & request);
	private:
		bool Awake() final;
		bool OnInit() final;
		void OnDisConnect(long long id);
	 private:
		class ActorComponent * mActorComponent;
		class OuterNetComponent* mOuterComponent;
	};

}

#endif //GAMEKEEPER_GATESERVICE_H
