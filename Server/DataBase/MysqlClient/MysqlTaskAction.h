#pragma once
#include"MysqlTaskBase.h"
#include<QueryResult/InvokeResultData.h>
namespace SoEasy
{
	class CoroutineManager;
	class MysqlTaskAction : public MysqlTaskBase
	{
	public:
		MysqlTaskAction(MysqlManager * mgr, long long id, const std::string & db, const std::string & sql, CoroutineManager * corMgr);
		~MysqlTaskAction() { }
	protected:
		void OnTaskFinish() final;
		void OnQueryFinish(QuertJsonWritre & jsonWriter) override; //查询完成之后调用
		bool GetSqlCommand(std::string & sql) final { sql = this->mSqlCommand; return true; }
	public:
		const long long GetCoroutienId() { return this->mCoroutineId; }
	public:
		std::shared_ptr<InvokeResultData> GetInvokeData();
	private:
		long long mCoroutineId;
		const std::string mSqlCommand;
		CoroutineManager * mCoroutineMgr;
		std::shared_ptr<rapidjson::Document> mDocument;
	};
	typedef std::shared_ptr<MysqlTaskAction> MysqlSharedTask;
}