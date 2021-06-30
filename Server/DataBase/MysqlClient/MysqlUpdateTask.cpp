#include "MysqlUpdateTask.h"
#include <Manager/MysqlManager.h>
#include <Coroutine/CoroutineManager.h>
namespace SoEasy
{

	MysqlUpdateTask::MysqlUpdateTask(MysqlManager *mgr, long long id, const std::string &db)
		: MysqlTaskBase(mgr, id, db)
	{
		this->mData = nullptr;
	}

	bool MysqlUpdateTask::InitTask(const std::string tab, CoroutineManager *corMgr, Message *data)
	{
		if (!corMgr->IsInMainCoroutine())
		{
			this->mData = data;
			this->mTable = tab;
			this->mCorManager = corMgr;
			SayNoAssertRetFalse_F(this->GetTabConfig(tab));
			this->mCoroutineId = corMgr->GetCurrentCorId();
			return true;
		}
		return false;
	}

	void MysqlUpdateTask::OnTaskFinish()
	{
		SayNoAssertRet_F(this->mCorManager);
		this->mCorManager->Resume(this->mCoroutineId);
	}

	bool MysqlUpdateTask::GetSqlCommand(std::string &sql)
	{
		if (!this->mSqlCommand.empty())
		{
			sql = this->mSqlCommand;
			return true;
		}
		std::stringstream sqlStream;
		std::stringstream sqlStream2;
		std::vector<const FieldDescriptor *> fieldList;
		const Descriptor *pDescriptor = this->mData->GetDescriptor();
		const Reflection *pReflection = this->mData->GetReflection();
		pReflection->ListFields(*this->mData, &fieldList);

		sqlStream2 << " where ";
		sqlStream << "update " << this->mTable << " set ";

		SqlTableConfig *tableConfig = this->GetTabConfig(this->mTable);
		for (size_t index = 0; index < tableConfig->mKeys.size(); index++)
		{
			const std::string &key = tableConfig->mKeys[index];
			const FieldDescriptor *fieldDesc = pDescriptor->FindFieldByName(key);
			sqlStream2 << key << "=";
			if (fieldDesc->type() == FieldDescriptor::TYPE_STRING)
			{
				std::string key = pReflection->GetString(*this->mData, fieldDesc);
				if (key == fieldDesc->default_value_string())
				{
					return false;
				}
				sqlStream2 << "'" << key << "'";
			}
			else if (fieldDesc->type() == FieldDescriptor::TYPE_INT64)
			{
				long long key = pReflection->GetInt64(*this->mData, fieldDesc);
				if (key == fieldDesc->default_value_int64())
				{
					return false;
				}
				sqlStream2 << key;
			}
			else if (fieldDesc->type() == FieldDescriptor::TYPE_UINT64)
			{
				unsigned long long key = pReflection->GetUInt64(*this->mData, fieldDesc);
				if (key == fieldDesc->default_value_uint64())
				{
					return false;
				}
				sqlStream2 << key;
			}

			else if (fieldDesc->type() == FieldDescriptor::TYPE_INT32)
			{
				int key = pReflection->GetInt32(*this->mData, fieldDesc);
				if (key == fieldDesc->default_value_int32())
				{
					return false;
				}
				sqlStream2 << key;
			}

			else if (fieldDesc->type() == FieldDescriptor::TYPE_UINT32)
			{
				unsigned int key = pReflection->GetUInt32(*this->mData, fieldDesc);
				if (key == fieldDesc->default_value_uint32())
				{
					return false;
				}
				sqlStream2 << key;
			}
			else
			{
				return false;
			}
			if (tableConfig->mKeys.size() > 1 && index < tableConfig->mKeys.size() - 1)
			{
				sqlStream2 << " and ";
			}
		}

		for (size_t index = 0; index < fieldList.size(); index++)
		{
			const FieldDescriptor *fieldDesc = fieldList[index];
			if (tableConfig->HasKey(fieldDesc->name()))
			{
				continue;
			}
			sqlStream << fieldDesc->name() << "=";
			switch (fieldDesc->type())
			{
			case FieldDescriptor::Type::TYPE_BYTES:
			case FieldDescriptor::Type::TYPE_STRING:
				sqlStream << "'" << pReflection->GetString(*this->mData, fieldDesc) << "',";
				break;
			case FieldDescriptor::Type::TYPE_INT64:
				sqlStream << pReflection->GetInt64(*this->mData, fieldDesc) << ",";
				break;
			case FieldDescriptor::Type::TYPE_UINT64:
				sqlStream << pReflection->GetUInt64(*this->mData, fieldDesc) << ",";
				break;
			case FieldDescriptor::Type::TYPE_INT32:
				sqlStream << pReflection->GetInt32(*this->mData, fieldDesc) << ",";
				break;
			case FieldDescriptor::Type::TYPE_UINT32:
				sqlStream << pReflection->GetUInt32(*this->mData, fieldDesc) << ",";
				break;
			case FieldDescriptor::Type::TYPE_FLOAT:
				sqlStream << pReflection->GetFloat(*this->mData, fieldDesc) << ",";
				break;
			case FieldDescriptor::Type::TYPE_DOUBLE:
				sqlStream << pReflection->GetDouble(*this->mData, fieldDesc) << ",";
				break;
			default:
				return false;
			}
		}
		this->mSqlCommand = sqlStream.str();
		this->mSqlCommand[this->mSqlCommand.size() - 1] = ' ';
		sql = this->mSqlCommand.append(sqlStream2.str());

		return true;
	}
}
