#pragma once
#include"RedisDefine.h"
#include<Thread/ThreadTaskAction.h>
#include<QueryResult/InvokeResultData.h>
namespace SoEasy
{
	class RedisTaskAction : public ThreadTaskAction
	{
	public:
		RedisTaskAction(RedisManager * mgr, long long taskId, long long corId);
	public:
		void InitCommand(const char *format, va_list command);
		void InvokeInThreadPool(long long threadId) override;	//在线程池执行的任务
		long long GetCoroutineId() { return mCoreoutineId; }
	public:
		std::shared_ptr<InvokeResultData> GetInvokeData();
	private:
		va_list mCommand;
		std::string mFormat;
		long long mCoreoutineId;
		RedisManager * mRedisManager;
	private:
		XCode mErrorCode;
		std::string mErrorString;
		std::shared_ptr<rapidjson::Document> mDocument;
	};
}