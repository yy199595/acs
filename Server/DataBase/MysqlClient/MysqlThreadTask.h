#pragma once
#include "MysqlDefine.h"
#include <Thread/ThreadTaskAction.h>
namespace SoEasy
{
	class SqlTableConfig;
	class QuertJsonWritre;
	class CoroutineManager;
	class MysqlThreadTask : public ThreadTaskAction
	{
	public:
		MysqlThreadTask(MysqlManager *mgr, const std::string &db, const std::string & sql);
		~MysqlThreadTask() {}
	protected:
		void OnTaskFinish() final;
		void InvokeInThreadPool(long long threadId) final; //在其他线程查询
	public:
		XCode GetErrorCode() { return this->mErrorCode; }
		const std::string &GetErrorStr() { return this->mErrorString; }
		const std::vector<std::string> &GetQueryDatas() { return this->mQueryDatas; }
	private:
		void WriteValue(QuertJsonWritre &jsonWriter, MYSQL_FIELD *field, const char *data, long size);
	private:
		const std::string mSqlCommand;
		const std::string mDataBaseName;
	private:
		XCode mErrorCode;
		long long mCoroutineId;
		std::string mErrorString;	
		MysqlManager *mMysqlManager;
		CoroutineManager * mCorManager;
		std::vector<std::string> mQueryDatas;
	private:
		double mValue1;
		long long mValue2;
	};
}