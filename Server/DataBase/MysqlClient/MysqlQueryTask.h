#pragma once
#include"MysqlTaskBase.h"
namespace SoEasy
{
	class CoroutineManager;
	class MysqlQueryTask : public MysqlTaskBase
	{
	public:
		MysqlQueryTask(MysqlManager * mgr, long long id, const std::string & db);
	public:
		bool InitTask(const std::string tab, CoroutineManager * corMgr, shared_ptr<Message> data) final;
	protected:
		void OnTaskFinish() final;
		const std::string & GetSqlCommand() final;
		void OnQueryFinish(QuertJsonWritre & jsonWriter) final; //查询完成之后调用
	private:
		std::string mTable;
		long long mCoroutineId;
		std::string mSqlCommand;
		shared_ptr<Message> mData;
		CoroutineManager * mCorManager;
	};
}