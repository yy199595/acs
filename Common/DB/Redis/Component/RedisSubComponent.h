//
// Created by leyi on 2024/1/18.
//

#ifndef APP_REDISSUBCOMPONENT_H
#define APP_REDISSUBCOMPONENT_H
#include"Redis/Client/Client.h"
#include"Redis/Config/RedisConfig.h"
#include"Entity/Component/Component.h"


namespace acs
{
	class RedisSubComponent final : public Component, public IRpc<redis::Request, redis::Response>
	{
	public:
		RedisSubComponent();
		~RedisSubComponent() final = default;
	public:
		int SubChannel(const std::string & chanel);
		int UnSubChannel(const std::string & chanel);
	private:
		bool Awake() final;
		bool LateAwake() final;
		void OnConnectOK(int id) final;
		void OnMessage(int, redis::Request *request, redis::Response *response) noexcept final;
	private:
		redis::Config mConfig;
		std::shared_ptr<redis::Client> mClient;
		std::unordered_map<std::string, int> mChannels;
	};
}


#endif //APP_REDISSUBCOMPONENT_H
