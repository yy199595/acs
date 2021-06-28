#include"MysqlUpdateTask.h"
#include<Manager/MysqlManager.h>
#include<Coroutine/CoroutineManager.h>
namespace SoEasy
{

	MysqlUpdateTask::MysqlUpdateTask(MysqlManager * mgr, long long id, const std::string & db)
		: MysqlTaskBase(mgr, id, db)
	{
		this->mData = nullptr;
	}

	bool MysqlUpdateTask::InitTask(const std::string tab, CoroutineManager * corMgr, Message * data)
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

	const std::string & MysqlUpdateTask::GetSqlCommand()
	{
		if (this->mSqlCommand.empty())
		{
			std::stringstream sqlStream;
			std::stringstream sqlStream2;
			std::vector<const FieldDescriptor *> fieldList;
			const Descriptor * pDescriptor = this->mData->GetDescriptor();
			const Reflection * pReflection = this->mData->GetReflection();
			pReflection->ListFields(*this->mData, &fieldList);

			sqlStream2 << " where ";
			sqlStream << "update " << this->mTable << " set ";

			SqlTableConfig * tableConfig = this->GetTabConfig(this->mTable);
			for (size_t index = 0; index < tableConfig->mKeys.size(); index++)
			{
				const std::string & key = tableConfig->mKeys[index];
				const FieldDescriptor * fieldDesc = pDescriptor->FindFieldByName(key);
				sqlStream2 << key << "=";
				switch (fieldDesc->type())
				{
				case FieldDescriptor::Type::TYPE_STRING:
					sqlStream2 << "'" << pReflection->GetString(*this->mData, fieldDesc) << "'";
					break;
				case FieldDescriptor::Type::TYPE_INT64:
					sqlStream2 << pReflection->GetInt64(*this->mData, fieldDesc);
					break;
				case FieldDescriptor::Type::TYPE_INT32:
					sqlStream2 << pReflection->GetInt32(*this->mData, fieldDesc);
					break;
				default:
					assert(false);
					break;
				}
				if (tableConfig->mKeys.size() > 1 && index < tableConfig->mKeys.size() - 1)
				{
					sqlStream2 << " and ";
				}
			}

			for (size_t index = 0; index < fieldList.size(); index++)
			{
				const FieldDescriptor * fieldDesc = fieldList[index];
				if (tableConfig->HasKey(fieldDesc->name()))
				{
					continue;
				}
				sqlStream << fieldDesc->name() << "=";
				switch (fieldDesc->type())
				{
				case FieldDescriptor::Type::TYPE_STRING:	
					sqlStream << "'" << pReflection->GetString(*this->mData, fieldDesc) << "',";
					break;
				case FieldDescriptor::Type::TYPE_INT64:
					sqlStream << pReflection->GetInt64(*this->mData, fieldDesc) << ",";
					break;
				case FieldDescriptor::Type::TYPE_INT32:
					sqlStream << pReflection->GetInt32(*this->mData, fieldDesc) << ",";
					break;
				case FieldDescriptor::Type::TYPE_FLOAT:
					sqlStream << pReflection->GetFloat(*this->mData, fieldDesc) << ",";
					break;
				case FieldDescriptor::Type::TYPE_DOUBLE:
					sqlStream << pReflection->GetDouble(*this->mData, fieldDesc) << ",";
					break;
				default:
					break;
				}				
			}
			this->mSqlCommand = sqlStream.str();
			this->mSqlCommand[this->mSqlCommand.size() - 1] = ' ';
			this->mSqlCommand.append(sqlStream2.str());
		}
		return this->mSqlCommand;
	}
}
