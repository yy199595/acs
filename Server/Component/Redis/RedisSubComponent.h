//
// Created by yjz on 2022/4/29.
//

#ifndef _REDISSUBCOMPONENT_H_
#define _REDISSUBCOMPONENT_H_
#include"Protocol/sub.pb.h"
#include"Component/Component.h"
#include"DB/Redis/RedisClientContext.h"
namespace Sentry
{
	class RedisSubComponent final : public Component, public IStart, public IComplete,
		public IProtoRpc<com::Rpc::Request, com::Rpc::Response>
	{
	 public:
		RedisSubComponent() = default;
		~RedisSubComponent() = default;
	 public:
		XCode OnRequest(std::shared_ptr<com::Rpc::Request> request);
		XCode OnResponse(std::shared_ptr<com::Rpc::Response> response);
	 private:
		bool LateAwake() final;
	 private:
		TaskComponent* mTaskComponent;
		const struct RedisConfig* mConfig;
		class MainRedisComponent* mMainRedisComponent;
		std::shared_ptr<RedisClientContext> mSubClient;
	};
}

#endif //_REDISSUBCOMPONENT_H_
