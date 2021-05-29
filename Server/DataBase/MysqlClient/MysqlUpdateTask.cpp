#include"MysqlUpdateTask.h"
#include<Coroutine/CoroutineManager.h>
namespace SoEasy
{

	MysqlUpdateTask::MysqlUpdateTask(MysqlManager * mgr, long long id, const std::string & db)
		: MysqlTaskBase(mgr, id, db)
	{

	}

	bool MysqlUpdateTask::InitTask(const std::string tab, CoroutineManager * corMgr, const std::string key, shared_ptr<Message> data)
	{
		if (!corMgr->IsInMainCoroutine())
		{
			this->mKey = key;
			this->mData = data;
			this->mTable = tab;
			this->mCorManager = corMgr;
			this->mCoroutineId = corMgr->GetCurrentCorId();
			const Descriptor * pDescriptor = this->mData->GetDescriptor();
			this->mFieldDesc = pDescriptor->FindFieldByName(this->mKey);
			return this->mFieldDesc != nullptr;
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

			sqlStream << "update " << this->mTable << " set ";
			sqlStream2 << "where " << this->mKey << "=";
			for (size_t index = 0; index < fieldList.size(); index++)
			{
				const FieldDescriptor * fieldDesc = fieldList[index];
				if (fieldDesc->type() == FieldDescriptor::Type::TYPE_STRING)
				{
					if (fieldDesc->name() != this->mKey)
					{
						sqlStream << fieldDesc->name() << "='";
						sqlStream << pReflection->GetString(*this->mData, fieldDesc) << "',";
						continue;
					}
					sqlStream2 << "'" << pReflection->GetString(*this->mData, fieldDesc) << "'";
				}
				else if (fieldDesc->type() == FieldDescriptor::Type::TYPE_INT64)
				{
					if (fieldDesc->name() != this->mKey)
					{
						sqlStream << fieldDesc->name() << "=";
						sqlStream << pReflection->GetInt64(*this->mData, fieldDesc) << ",";
						continue;
					}
					sqlStream2 << pReflection->GetInt64(*this->mData, fieldDesc);
				}
				else if (fieldDesc->type() == FieldDescriptor::Type::TYPE_INT32)
				{
					if (fieldDesc->name() != this->mKey)
					{
						sqlStream << fieldDesc->name() << "=";
						sqlStream << pReflection->GetInt32(*this->mData, fieldDesc) << ",";
						continue;
					}
					sqlStream2 << pReflection->GetInt32(*this->mData, fieldDesc);
				}
				else if (fieldDesc->type() == FieldDescriptor::Type::TYPE_FLOAT)
				{
					sqlStream << fieldDesc->name() << "=";
					sqlStream << pReflection->GetFloat(*this->mData, fieldDesc) << ",";
				}
				else if (fieldDesc->type() == FieldDescriptor::Type::TYPE_DOUBLE)
				{
					sqlStream << fieldDesc->name() << "=";
					sqlStream << pReflection->GetDouble(*this->mData, fieldDesc) << ",";
				}
			}
			this->mSqlCommand = sqlStream.str();
			this->mSqlCommand[this->mSqlCommand.size() - 1] = ' ';
			this->mSqlCommand.append(sqlStream2.str());
		}
		return this->mSqlCommand;
	}
}
