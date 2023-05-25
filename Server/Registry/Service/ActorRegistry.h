//
// Created by yjz on 2023/5/25.
//

#ifndef _ACTORREGISTRY_H_
#define _ACTORREGISTRY_H_
#include"Rpc/Service/RpcService.h"
#include"Message/s2s/s2s.pb.h"
namespace Tendo
{
	class ActorRegistry : public RpcService
	{
	 public:
		bool Awake() final;
		bool OnInit() final;
	 private:
		int Query(const s2s::registry::query & request);
		int Register(const s2s::registry::request & request);
	 private:
		class RedisComponent * mRedisComponent;
		std::unordered_map<std::string, std::unordered_map<long long, std::string>> mRegistrys;
	};
}

#endif //_ACTORREGISTRY_H_
