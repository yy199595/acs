#pragma once
#include"MysqlTaskBase.h"
namespace SoEasy
{
	class CoroutineManager;
	class MysqlInsertTask : public MysqlTaskBase
	{
	public:
		MysqlInsertTask(MysqlManager * mgr, long long id, const std::string & db);
	public:
		bool InitTask(const std::string tab, CoroutineManager * corMgr, Message * data) final;
	protected:
		void OnTaskFinish() final;
		const std::string & GetSqlCommand() final;
	private:
		Message * mData;
		std::string mTable;
		long long mCoroutineId;
		std::string mSqlCommand;
		CoroutineManager * mCorManager;
	};
}