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
		const XCode GetCode() { return this->mErrorCode; }
		const std::string & GetErrorStr() { return this->mErrorString; }
		const std::string & GetJsonData() { return this->mQuertJsonData; }
	private:
		va_list mCommand;
		std::string mFormat;
		long long mCoreoutineId;
		RedisManager * mRedisManager;
	private:
		XCode mErrorCode;
		std::string mErrorString;
		std::string mQuertJsonData;
	};
}