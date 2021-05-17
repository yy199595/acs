#pragma once
#include"RedisTaskBase.h"
#include<QueryResult/InvokeResultData.h>
namespace SoEasy
{
	class CoroutineManager;
	class RedisTaskAction : public RedisTaskBase
	{
	public:
		RedisTaskAction(RedisManager * mgr, long long taskId, const std::string & cmd,CoroutineManager * corMgr);
	public:
		long long GetCoroutineId() { return mCoreoutineId; }
	protected:
		void OnTaskFinish() final;  //执行完成之后在主线程调用
		void OnQueryFinish(QuertJsonWritre & jsonWriter) final;
	public:
		std::shared_ptr<InvokeResultData> GetInvokeData();
	private:
		long long mCoreoutineId;
		CoroutineManager * mCoroutineManager;
	private:
		std::shared_ptr<rapidjson::Document> mDocument;
	};
}