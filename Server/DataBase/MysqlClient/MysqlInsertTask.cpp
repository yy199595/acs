#include "MysqlInsertTask.h"
#include <Coroutine/CoroutineManager.h>
namespace SoEasy
{
	MysqlInsertTask::MysqlInsertTask(MysqlManager *mgr, long long id, const std::string &db)
		: MysqlTaskBase(mgr, id, db)
	{
		this->mData = nullptr;
	}

	bool MysqlInsertTask::InitTask(const std::string tab, CoroutineManager *corMgr, Message *data)
	{
		if (!corMgr->IsInMainCoroutine())
		{
			this->mData = data;
			this->mTable = tab;
			this->mCorManager = corMgr;
			this->mCoroutineId = corMgr->GetCurrentCorId();
			return true;
		}
		return false;
	}

	void MysqlInsertTask::OnTaskFinish()
	{
		SayNoAssertRet_F(this->mCorManager);
		this->mCorManager->Resume(this->mCoroutineId);
	}

	bool MysqlInsertTask::GetSqlCommand(std::string &sql)
	{
		if (!this->mSqlCommand.empty())
		{
			sql = this->mSqlCommand;
			return true;
		}

		std::stringstream sqlStream;
		std::vector<const FieldDescriptor *> fieldList;
		const Descriptor *pDescriptor = this->mData->GetDescriptor();
		const Reflection *pReflection = this->mData->GetReflection();
		pReflection->ListFields(*this->mData, &fieldList);

		sqlStream << "insert into " << this->mTable << "(";
		for (size_t index = 0; index < fieldList.size(); index++)
		{
			if (index < fieldList.size() - 1)
			{
				sqlStream << fieldList[index]->name() << ",";
				continue;
			}
			sqlStream << fieldList[index]->name();
		}
		sqlStream << ")values(";

		for (size_t index = 0; index < fieldList.size(); index++)
		{
			const FieldDescriptor * fieldDesc = fieldList[index];
			if (fieldDesc->type() == FieldDescriptor::Type::TYPE_STRING)
			{
				sqlStream << "'" << pReflection->GetString(*this->mData, fieldDesc) << "',";
			}
			else if(fieldDesc->type() == FieldDescriptor::Type::TYPE_BYTES)
			{
				sqlStream << "'" << pReflection->GetString(*this->mData, fieldDesc) << "',";				
			}
			else if (fieldDesc->type() == FieldDescriptor::Type::TYPE_INT64)
			{
				sqlStream << pReflection->GetInt64(*this->mData, fieldDesc) << ",";
			}
			else if (fieldDesc->type() == FieldDescriptor::Type::TYPE_UINT64)
			{
				sqlStream << pReflection->GetUInt64(*this->mData, fieldDesc) << ",";
			}
			else if (fieldDesc->type() == FieldDescriptor::Type::TYPE_INT32)
			{
				sqlStream << pReflection->GetInt32(*this->mData, fieldDesc) << ",";
			}
			else if (fieldDesc->type() == FieldDescriptor::Type::TYPE_UINT32)
			{
				sqlStream << pReflection->GetUInt32(*this->mData, fieldDesc) << ",";
			}
			else if (fieldDesc->type() == FieldDescriptor::Type::TYPE_FLOAT)
			{
				sqlStream << pReflection->GetFloat(*this->mData, fieldDesc) << ",";
			}
			else if (fieldDesc->type() == FieldDescriptor::Type::TYPE_DOUBLE)
			{
				sqlStream << pReflection->GetDouble(*this->mData, fieldDesc) << ",";
			}
			else
			{
				return false;
			}
		}
		this->mSqlCommand = sqlStream.str();
		this->mSqlCommand[this->mSqlCommand.size() - 1] = ')';
		sql = this->mSqlCommand;
		return true;
	}
}