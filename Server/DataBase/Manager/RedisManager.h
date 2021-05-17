#pragma once
#include<Coroutine/CoroutineManager.h>
#include<QueryResult/InvokeResultData.h>
#include<RedisClient/RedisTaskAction.h>
using namespace SoEasy;
namespace SoEasy
{
	class RedisManager : public Manager
	{
	public:
		RedisManager();
		~RedisManager() { }
		RedisSocket * GetRedisSocket(long long id);
	protected:
		bool OnInit() final;			//初始化管理器
		void OnInitComplete() override;				//初始化完成之后
		void PushClassToLua(lua_State * luaEnv) final;		//自身方法导出到lua
	public:
		template<typename ... Args>
		shared_ptr<InvokeResultData> InvokeCommand(const char * cmd, Args && ... args);
	public:
		bool HasValue(const char * key);
		bool HasValue(const char * tab, const char * key);
	public:
		bool DelValue(const char * key);
		bool DelValue(const char * tab, const char * key);
	public:
		bool SetValue(const char * key, const std::string & value);
		bool SetValue(const char * key, const std::string & value, int second);
		bool SetValue(const char * tab, const char * key, const std::string & value);
		bool SetValue(const char * tab, const char * key, const shared_ptr<Message> value);
	public:
		bool GetValue(const char * key, std::string & value);
		bool GetValue(const char * tab, const char * key, std::string & value);
		bool GetValue(const char * tab, const char * key, shared_ptr<Message> value);
	private:
		std::string mRedisIp;		//redis ip地址
		unsigned short mRedisPort;	//端口号
		ThreadPool * mThreadPool;	//线程池
		class CoroutineManager * mCoroutineScheduler;
		std::unordered_map<long long, redisContext *> mRedisContextMap;
	};
	template<typename ...Args>
	inline shared_ptr<InvokeResultData> RedisManager::InvokeCommand(const char * cmd, Args && ...args)
	{
		if (this->mCoroutineScheduler->IsInMainCoroutine())
		{
			SayNoDebugError("[redis error] redis not in coreoutine");
			return make_shared<InvokeResultData>(XCode::RedisNotInCoroutine);
		}

		long long taskActionId = NumberHelper::Create();
		shared_ptr<RedisTaskAction> taskAction = make_shared<RedisTaskAction>(this, taskActionId, cmd, this->mCoroutineScheduler);
		taskAction->InitCommand(std::forward<Args>(args)...);	
		if (!this->StartTaskAction(taskAction))
		{
			SayNoDebugError("[redis error] start redis task fail");
			return make_shared<InvokeResultData>(XCode::RedisStartTaskFail);
		}

		this->mCoroutineScheduler->YieldReturn();

		return taskAction->GetInvokeData();
	}
}