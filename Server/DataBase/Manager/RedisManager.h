#pragma once
#include<Manager/Manager.h>
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
		void OnTaskFinish(long long id) final;		//在线程池完成任务之后的通知
	public:
		shared_ptr<InvokeResultData> InvokeCommand(const char * format, ...);
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
		std::unordered_map<long long, shared_ptr<RedisTaskAction>> mTaskActionMap;
	};
}