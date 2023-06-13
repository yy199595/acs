//
// Created by yjz on 2023/5/25.
//

#ifndef _ACTORREGISTRY_H_
#define _ACTORREGISTRY_H_
#include"Rpc/Service/RpcService.h"
#include"Message/s2s/registry.pb.h"
#include"Registry/Target/TargetRegistry.h"
namespace Tendo
{
	class ActorRegistry : public RpcService
	{
	 public:
		bool Awake() final;
		bool OnInit() final;
	 private:
		int Add(const registry::actor & request);
		int Del(const registry::actor & request);
		int Query(const registry::query::request & request, registry::query::response &response);
	private:
		TargetRegistry * mTarget;
	};
}

#endif //_ACTORREGISTRY_H_
