#pragma once
#include"MysqlTaskBase.h"
namespace SoEasy
{
	class CoroutineManager;
	class MysqlDeleteTask : public MysqlTaskBase
	{
	public:
		MysqlDeleteTask(MysqlManager * mgr, long long id, const std::string & db);
		~MysqlDeleteTask() { }
	public:
		bool InitTask(const std::string tab, CoroutineManager * corMgr, shared_ptr<Message> data) final;
	protected:
		void OnTaskFinish() final;
		const std::string & GetSqlCommand() final;
	private:
		std::string mTable;
		long long mCoroutineId;
		std::string mSqlCommand;
		shared_ptr<Message> mData;
		CoroutineManager * mCorManager;
		const FieldDescriptor * mFieldDesc;
	};
}