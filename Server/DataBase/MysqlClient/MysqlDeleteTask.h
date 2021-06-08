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
		bool InitTask(const std::string tab, const std::string & key, CoroutineManager * corMgr, shared_ptr<Message> data);
	protected:
		void OnTaskFinish() final;
		const std::string & GetSqlCommand() final;
	private:
		std::string mKey;
		std::string mTable;
		long long mCoroutineId;
		std::string mSqlCommand;
		shared_ptr<Message> mData;
		CoroutineManager * mCorManager;
		const FieldDescriptor * mFieldDesc;
	};
}