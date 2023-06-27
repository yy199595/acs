//
// Created by yjz on 2023/5/25.
//

#ifndef _ACTORREGISTRY_H_
#define _ACTORREGISTRY_H_
#include"Rpc/Service/RpcService.h"
#include"Message/com/com.pb.h"
#include"Message/s2s/registry.pb.h"
#include"Registry/Target/TargetRegistryComponent.h"
namespace Tendo
{
	//服务器注册服务
	class ServerRegistry : public RpcService
	{
	 public:
		ServerRegistry();
	 public:
		bool Awake() final;
		bool OnInit() final;
	 private:
		int Del(long long id);
		int Add(long long id, const com::type::json & request);
		int Watch(long long id, const com::array::string & request);
		int Query(const registry::query::request & request, registry::query::response &response);
	private:
		const std::string mTable;
		TargetRegistryComponent * mTarget;
	};
}

#endif //_ACTORREGISTRY_H_
