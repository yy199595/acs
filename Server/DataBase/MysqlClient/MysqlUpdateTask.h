#pragma once
#include"MysqlTaskBase.h"
namespace SoEasy
{
	class CoroutineManager;
	class MysqlUpdateTask : public MysqlTaskBase
	{
	public:
		MysqlUpdateTask(MysqlManager * mgr, long long id, const std::string & db);
		~MysqlUpdateTask() { }
	public:
		bool InitTask(const std::string tab, CoroutineManager * corMgr, Message * data) final;
	protected:
		void OnTaskFinish() final;
		bool GetSqlCommand(std::string & sql) final;
	private:
		Message * mData;
		std::string mTable;
		long long mCoroutineId;
		std::string mSqlCommand;
		CoroutineManager * mCorManager;
	};
}