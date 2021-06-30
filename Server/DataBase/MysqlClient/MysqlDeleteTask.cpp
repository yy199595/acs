#include "MysqlDeleteTask.h"
#include <Manager/MysqlManager.h>
#include <Coroutine/CoroutineManager.h>
namespace SoEasy
{
	MysqlDeleteTask::MysqlDeleteTask(MysqlManager *mgr, long long id, const std::string &db)
		: MysqlTaskBase(mgr, id, db)
	{
		this->mData = nullptr;
	}
	bool MysqlDeleteTask::InitTask(const std::string tab, CoroutineManager *corMgr, Message *data)
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

	bool MysqlDeleteTask::GetSqlCommand(std::string &sql)
	{
		if (!this->mSqlCommand.empty())
		{
			sql = this->mSqlCommand;
			return true;
		}

		std::stringstream sqlStream;
		sqlStream << "delete from " << this->mTable << " where ";
		const Descriptor *pDescriptor = this->mData->GetDescriptor();
		const Reflection *pReflection = this->mData->GetReflection();
		SqlTableConfig *tableConfig = this->GetTabConfig(this->mTable);
		for (size_t index = 0; index < tableConfig->mKeys.size(); index++)
		{
			const std::string &key = tableConfig->mKeys[index];
			const FieldDescriptor *fieldDesc = pDescriptor->FindFieldByName(key);
			sqlStream << key << "=";
			if (fieldDesc->type() == FieldDescriptor::Type::TYPE_STRING)
			{
				const std::string key = pReflection->GetString(*this->mData, fieldDesc);
				if (key == fieldDesc->default_value_string())
				{
					return false;
				}
				sqlStream << "'" << key << "'";
			}
			else if(fieldDesc->type() == FieldDescriptor::Type::TYPE_INT32)
			{
				int key = pReflection->GetInt32(*this->mData, fieldDesc);
				if(key == fieldDesc->default_value_int32())
				{
					return false;
				}
				sqlStream << key;
			}
			else if(fieldDesc->type() == FieldDescriptor::TYPE_UINT32)
			{
				unsigned int key = pReflection->GetUInt32(*this->mData, fieldDesc);
				if(key == fieldDesc->default_value_uint32())
				{
					return false;
				}
				sqlStream << key;
			}
			else if(fieldDesc->type() == FieldDescriptor::Type::TYPE_INT64)
			{
				long long key = pReflection->GetInt64(*this->mData, fieldDesc);
				if(key == fieldDesc->default_value_int64())
				{
					return false;
				}
				sqlStream << key;
			}
			else if(fieldDesc->type() == FieldDescriptor::TYPE_UINT64)
			{
				unsigned long long key = pReflection->GetUInt64(*this->mData, fieldDesc);
				if(key == fieldDesc->default_value_uint64())
				{
					return false;
				}
				sqlStream << key;
			}
			else
			{
				return false;
			}

			if (tableConfig->mKeys.size() > 1 && index < tableConfig->mKeys.size() - 1)
			{
				sqlStream << " and ";
			}
		}
		sql = this->mSqlCommand = sqlStream.str();
		return true;
	}
}