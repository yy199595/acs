#pragma once
#include<Manager/Manager.h>
#include<RedisClient/RedisTaskAction.h>
using namespace SoEasy;
namespace DataBase
{
	class RedisManager : public Manager
	{
	public:
		RedisManager();
		~RedisManager() { }
		RedisSocket * GetRedisSocket(long long id);
	protected:
		bool OnInit() final;			//初始化管理器
		void OnSecondUpdate() override;				//每秒调用
	private:
		std::string mRedisIp;		//redis ip地址
		unsigned short mRedisPort;	//端口号
		ThreadPool * mThreadPool;	//线程池
		class CoroutineManager * mCoroutineScheduler;
		std::unordered_map<long long, redisContext *> mRedisContextMap;
	};
}