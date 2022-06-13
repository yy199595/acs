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

	class RedisComponent : public Component, public IStart
	{
	public:
		RedisComponent() = default;

	public:
		bool CallLua(const std::string & name, const std::string& fullName, const std::string & json);

		bool CallLua(const std::string & name,const std::string& fullName, const std::string & json,
				std::shared_ptr<Json::Reader> response);
	 public:
		RedisClientContext * GetClient(const std::string & name);
		void PushClient(RedisClientContext * redisClientContext);
		RedisClientContext * MakeRedisClient(const RedisConfig* config);
		bool TryAsyncConnect(RedisClientContext * client, int maxCount = 0);
	protected:
		bool OnStart() override;

	 private:
		bool LateAwake() final;
		const RedisConfig * GetRedisConfig(const std::string & name);
		bool LoadLuaScript(const std::string & redis, const std::string & path);
	private:
		std::unordered_map<std::string, RedisConfig> mConfigs;
		std::unordered_map<std::string, std::string> mLuaMap;
		std::unordered_map<std::string, std::queue<RedisClientContext *>> mRedisClients;
	};
}


#endif //SERVER_REDISBASECOMPONENT_H
