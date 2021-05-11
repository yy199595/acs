#pragma once
#include"RedisDefine.h"
#include<Thread/ThreadTaskAction.h>
namespace SoEasy
{
	class RedisTaskAction : public ThreadTaskAction
	{
	public:
		RedisTaskAction(RedisManager * mgr, long long id, const std::string & command);
	public:
		void InvokeInThreadPool(long long threadId) override;	//在线程池执行的任务
	private:
		std::string mCommand;
		RedisManager * mRedisManager;
	};
}