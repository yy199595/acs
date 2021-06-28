#include "MysqlDeleteTask.h"
#include<Manager/MysqlManager.h>
#include<Coroutine/CoroutineManager.h>
namespace SoEasy
{
	MysqlDeleteTask::MysqlDeleteTask(MysqlManager * mgr, long long id, const std::string & db)
		: MysqlTaskBase(mgr, id, db)
	{
		this->mData = nullptr;
	}
	bool MysqlDeleteTask::InitTask(const std::string tab, CoroutineManager * corMgr, Message * data)
	{
		SayNoAssertRetFalse_F(corMgr->IsInLogicCoroutine());
		this->mData = data;
		this->mTable = tab;
		this->mCorManager = corMgr;
		SayNoAssertRetFalse_F(this->GetTabConfig(tab));
		this->mCoroutineId = corMgr->GetCurrentCorId();
		return true;
	}
	void MysqlDeleteTask::OnTaskFinish()
	{
		SayNoAssertRet_F(this->mCorManager);
		this->mCorManager->Resume(this->mCoroutineId);
	}

	const std::string & MysqlDeleteTask::GetSqlCommand()
	{
		if (this->mSqlCommand.empty())
		{
			std::stringstream sqlStream;
			sqlStream << "delete from " << this->mTable << " where ";
			const Descriptor * pDescriptor = this->mData->GetDescriptor();
			const Reflection * pReflection = this->mData->GetReflection();
			SqlTableConfig * tableConfig = this->GetTabConfig(this->mTable);
			for (size_t index = 0; index < tableConfig->mKeys.size(); index++)
			{
				const std::string & key = tableConfig->mKeys[index];
				const FieldDescriptor * fieldDesc = pDescriptor->FindFieldByName(key);
				sqlStream << key << "=";
				switch (fieldDesc->type())
				{
				case FieldDescriptor::Type::TYPE_STRING:
					sqlStream << "'" << pReflection->GetString(*this->mData, fieldDesc) << "'";
					break;
				case FieldDescriptor::Type::TYPE_INT64:
					sqlStream << pReflection->GetInt64(*this->mData, fieldDesc);
					break;
				case FieldDescriptor::Type::TYPE_INT32:
					sqlStream << pReflection->GetInt32(*this->mData, fieldDesc);
					break;
				default:
					assert(false);
					break;
				}
				if (tableConfig->mKeys.size() > 1 && index < tableConfig->mKeys.size() - 1)
				{
					sqlStream << " and ";
				}
			}
			this->mSqlCommand = sqlStream.str();
		}
		return this->mSqlCommand;
	}
}