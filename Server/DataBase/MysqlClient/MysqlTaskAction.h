#pragma once
#include"MysqlDefine.h"
#include<Thread/ThreadTaskAction.h>
namespace SoEasy
{
	class MysqlTaskAction : public ThreadTaskAction
	{
	public:
		MysqlTaskAction(MysqlManager * mgr, long long id, long long corId, const std::string & db, const std::string & sql);
		~MysqlTaskAction() { }
	public:
		void InvokeInThreadPool(long long threadId);	//在其他线程查询
	public:
		const long long GetCoroutienId() { return this->mCoroutineId; }
		const std::string & GetSqlCommand() { return this->mSqlCommand; }
		const std::string & GetDataBaseName() { return this->mDataBaseName; }
	public:
		XCode GetCode() { return this->mErrorCode; }
		const std::string & GetErrorStr() { return this->mErrorString; }
		const std::string & GetJsonData() { return this->mQuertJsonData; }
	private:
		long long mCoroutineId;
		std::string mSqlCommand;
		std::string mDataBaseName;
		MysqlManager * mMysqlManager;
	private:
		XCode mErrorCode;
		std::string mErrorString;
		std::string mQuertJsonData;
	};
	typedef std::shared_ptr<MysqlTaskAction> MysqlSharedTask;
}