#pragma once

#include"Util/Guid.h"
#include"Json/JsonWriter.h"
#include"Other/ElapsedTimer.h"
#include"Component/Component.h"
#include"DB/Redis/RedisClient.h"
#include"Component/Coroutine/TaskComponent.h"
using namespace Sentry;
namespace Sentry
{
	class ThreadPoolComponent;

	class MainRedisComponent final : public Component, public IStart
	{
	 public:
		MainRedisComponent() = default;
		~MainRedisComponent() final = default;
	 protected:
		bool LateAwake() final;                //初始化完成之后
		bool OnStart() override;
	 private:
		void StartPubSub();
		bool LoadRedisConfig();
		void SubscribeMessage();
		void CheckRedisClient();
	public:
		bool Lock(const std::string & key);
		bool UnLock(const std::string & key);
	 public:
		long long AddCounter(const std::string& key);
		bool SubscribeChannel(const std::string& channel);
		long long Publish(const std::string& channel, Json::Writer& jsonWriter);
		long long Publish(const std::string& channel, const std::string& message);
		long long Publish(const std::string address, const std::string& func, Json::Writer& jsonWriter);
	 public:
		template<typename ... Args>
		std::shared_ptr<RedisResponse> InvokeCommand(const std::string& cmd, Args&& ... args)
		{
			std::shared_ptr<RedisRequest> request =
				std::make_shared<RedisRequest>(cmd);
			request->InitParameter(std::forward<Args>(args) ...);
			return this->InvokeCommand(request);
		}

		std::shared_ptr<RedisResponse> InvokeCommand(std::shared_ptr<RedisRequest> request);

		template<typename ... Args>
		std::shared_ptr<RedisResponse> Call(const std::string& func, Args&& ...args)
		{
			std::string script;
			int size = sizeof ...(Args) + 1;
			const size_t pos = func.find('.');
			if (pos == std::string::npos)
			{
				return nullptr;
			}
			std::string tab = func.substr(0, pos);
			std::string file = fmt::format("{0}.lua", tab);
			if (!this->GetLuaScript(file, script))
			{
				LOG_ERROR("not find redis script " << tab << ".lua");
				return nullptr;
			}
			string method = func.substr(pos + 1);
			return this->InvokeCommand("EVALSHA", script, size, method, std::forward<Args>(args)...);
		}

		std::shared_ptr<RedisResponse> Call(const std::string& func, std::vector<std::string>& args);

	 private:
		void OnLockTimeout(const std::string & name);
		std::shared_ptr<RedisClient> MakeRedisClient();
		std::shared_ptr<RedisClient> AllotRedisClient();
		bool GetLuaScript(const std::string& file, std::string& command);
		bool HandlerSubMessage(const std::string & channel, const std::string & message);
	 private:
		RedisConfig mConfig;
		std::string mRpcAddress;
		TaskComponent* mTaskComponent;
		TimerComponent * mTimerComponent;
		std::shared_ptr<RedisClient> mSubRedisClient;
		std::queue<std::shared_ptr<RedisClient>> mFreeClients;
		std::unordered_map<std::string, std::string> mLuaCommandMap;
		std::queue<TaskSourceShared<RedisClient>> mWaitAllotClients;
		std::unordered_map<std::string, long long> mLockTimers; //分布式锁的续命定时器
	};
}