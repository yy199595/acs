#pragma once
#include"MysqlDefine.h"
#include<Thread/ThreadTaskAction.h>
#include<QueryResult/InvokeResultData.h>
namespace SoEasy
{
	class CoroutineManager;
	class MysqlTaskAction : public ThreadTaskAction
	{
	public:
		MysqlTaskAction(MysqlManager * mgr, long long id, CoroutineManager * corMgr, const std::string & db, const std::string & sql);
		~MysqlTaskAction() { }
	public:
		void OnTaskFinish() final;
		void InvokeInThreadPool(long long threadId) final;	//在其他线程查询
	public:
		const long long GetCoroutienId() { return this->mCoroutineId; }
		const std::string & GetSqlCommand() { return this->mSqlCommand; }
		const std::string & GetDataBaseName() { return this->mDataBaseName; }
	public:
		std::shared_ptr<InvokeResultData> GetInvokeData();
	private:
		long long mCoroutineId;
		std::string mSqlCommand;
		std::string mDataBaseName;
		MysqlManager * mMysqlManager;
		CoroutineManager * mCoroutineMgr;
	private:
		XCode mErrorCode;
		std::string mErrorString;
		std::shared_ptr<rapidjson::Document> mDocument;
	};
	typedef std::shared_ptr<MysqlTaskAction> MysqlSharedTask;
}