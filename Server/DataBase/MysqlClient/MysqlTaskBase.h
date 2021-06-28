#pragma once
#include"MysqlDefine.h"
#include<Thread/ThreadTaskAction.h>
namespace SoEasy
{
	class SqlTableConfig;
	class QuertJsonWritre;
	class CoroutineManager;
	class MysqlTaskBase : public ThreadTaskAction
	{
	public:
		MysqlTaskBase(MysqlManager * mgr, long long id, const std::string & db);
		~MysqlTaskBase() { }
	protected:
		void InvokeInThreadPool(long long threadId) final;	//在其他线程查询
		virtual const std::string & GetSqlCommand() = 0;
		virtual void OnQueryFinish(QuertJsonWritre & jsonWriter); //查询完成之后调用
	public:
		const std::string & GetDataBaseName() { return this->mDataBaseName; }
		virtual bool InitTask(const std::string tab, CoroutineManager * corMgr, shared_ptr<Message> data) { }
	public:
		XCode GetErrorCode() { return this->mErrorCode; }
		SqlTableConfig * GetTabConfig(const std::string & tab);
		const std::string & GetErrorStr() { return this->mErrorString; }
	private:
		void WriteValue(QuertJsonWritre & jsonWriter, MYSQL_FIELD * field, const char * data, long size);
	private:
		std::string mDataBaseName;
		MysqlManager * mMysqlManager;
	private:
		XCode mErrorCode;
		std::string mErrorString;
		SqlTableConfig * mTableConfig;
	};
}