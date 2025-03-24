//
// Created by leyi on 2024/1/18.
//

#ifndef APP_REDISSUBCOMPONENT_H
#define APP_REDISSUBCOMPONENT_H
#include"Redis/Client/RedisClient.h"
#include"Redis/Config/RedisConfig.h"
#include"Entity/Component/Component.h"
#include "Rpc/Component/RpcComponent.h"


namespace acs
{
	class RedisSubComponent final : public RpcComponent<redis::Response>,
									public IRpc<redis::Request, redis::Response>
	{
	public:
		RedisSubComponent();
		~RedisSubComponent() final = default;
	public:
		bool Sub(const std::string & chanel);
		bool UnSub(const std::string & chanel);
		void Send(std::unique_ptr<redis::Request> request, int & rpcId);
	private:
		bool Awake() final;
		bool LateAwake() final;
		void OnConnectOK(int id) final;
		void OnClientError(int id, int code) final;
		void OnMessage(int, redis::Request *request, redis::Response *response) noexcept final;
	private:
		redis::Cluster mConfig;
		class TimerComponent * mTimer;
		class DispatchComponent * mDispatch;
		std::shared_ptr<redis::Client> mClient;
		std::unordered_set<std::string> mChannels;
	};
}


#endif //APP_REDISSUBCOMPONENT_H
