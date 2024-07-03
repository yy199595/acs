//
// Created by yjz on 2023/5/25.
//

#ifndef APP_MASTER_H
#define APP_MASTER_H
#include"Core/Map/HashMap.h"
#include"Message/com/com.pb.h"
#include"Message/s2s/registry.pb.h"
#include"Http/Service/HttpService.h"
#include"Async/Coroutine/CoroutineLock.h"
namespace joke
{
	//服务器注册服务
	class Master : public HttpService, public ISecondUpdate
	{
	 public:
		Master();
	private:
		bool OnInit() final;
		void OnSecondUpdate(int tick) final;
	 private:
		void OnDisConnect(int id);
		int Del(const http::FromData & request, json::w::Value & res);
		int Push(const json::r::Document & request, json::w::Value & res);
		int Find(const http::FromData & request, json::w::Document & response);
	private:
		std::string mToken;
		class RedisComponent * mRedis;
		std::unique_ptr<CoroutineLock> mLock;
		std::vector<std::string> mConfServers;
		class ActorComponent * mActorComponent;
		custom::HashMap<long long, std::string> mServerData;
	};
}

#endif //APP_MASTER_H
