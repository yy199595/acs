//
// Created by mac on 2022/5/18.
//

#ifndef SERVER_REDISBASECOMPONENT_H
#define SERVER_REDISBASECOMPONENT_H
#include"Component/Component.h"
#include"DB/Redis/RedisClientContext.h"
namespace Sentry
{
	struct RedisConfig;

	class RedisBaseComponent : public Component
	{
	public:
		RedisBaseComponent() = default;

	public:
		bool CallLua(const std::string& fullName, Json::Writer& request, const std::string redis = "");

		bool CallLua(const std::string& fullName, Json::Writer& request,
				std::shared_ptr<Json::Reader> response, const std::string redis = "");

	protected:
		const RedisConfig* LoadRedisConfig(const std::string& name);
		bool LoadLuaScript(const std::string & redis, const std::string & path);
		std::shared_ptr<RedisClientContext> MakeRedisClient(const RedisConfig* config);

		bool TryReConnect(std::shared_ptr<RedisClientContext> client, int maxCount = 0);

	protected:
		virtual std::shared_ptr<RedisClientContext> GetClient(const std::string& name) = 0;
	private:
		std::unordered_map<std::string, std::string> mLuaMap;
	};
}


#endif //SERVER_REDISBASECOMPONENT_H
