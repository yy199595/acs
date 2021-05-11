#pragma once
#include"MysqlDefine.h"
#include<Thread/ThreadTaskAction.h>
namespace SoEasy
{
	class MysqlTaskAction : public ThreadTaskAction
	{
	public:
		MysqlTaskAction(MysqlManager * mgr, long long id, const std::string & db, const std::string & sql);
		~MysqlTaskAction() { }
	public:
		void ResetTask(MysqlManager * mgr, long long id, const std::string & db, const std::string & sql);
	public:
		void InvokeInThreadPool(long long threadId);	//在其他线程查询
	public:
		long long GetActionId() { return this->mActionId; }
		const std::string & GetSqlCommand() { return this->mSqlCommand; }
		const std::string & GetDataBaseName() { return this->mDataBaseName; }
		std::shared_ptr<MysqlQueryData> GetQueryData() { return mMysqlQueryData; }
	private:
		bool SetCode(XCode code);
	private:
		long long mActionId;
		std::string mSqlCommand;
		std::string mDataBaseName;
		MysqlManager * mMysqlManager;
		std::shared_ptr<MysqlQueryData> mMysqlQueryData;
	};
	typedef std::shared_ptr<MysqlTaskAction> MysqlSharedTask;
}