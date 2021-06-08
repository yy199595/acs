#include "MysqlDeleteTask.h"
#include<Coroutine/CoroutineManager.h>
namespace SoEasy
{
	MysqlDeleteTask::MysqlDeleteTask(MysqlManager * mgr, long long id, const std::string & db)
		: MysqlTaskBase(mgr, id, db)
	{

	}
	bool MysqlDeleteTask::InitTask(const std::string tab, const std::string & key, CoroutineManager * corMgr, shared_ptr<Message> data)
	{
		SayNoAssertRetFalse_F(corMgr->IsInLogicCoroutine());

		this->mKey = key;
		this->mData = data;
		this->mTable = tab;
		this->mCorManager = corMgr;
		this->mCoroutineId = corMgr->GetCurrentCorId();
		const Descriptor * pDescriptor = this->mData->GetDescriptor();
		this->mFieldDesc = pDescriptor->FindFieldByName(this->mKey);
		return this->mFieldDesc != nullptr;
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
			sqlStream << "delete from " << this->mTable << " where " << this->mKey << "=";
			const Reflection * pReflection = this->mData->GetReflection();
			if (this->mFieldDesc->type() == FieldDescriptor::Type::TYPE_STRING)
			{
				sqlStream << "'" << pReflection->GetString(*this->mData, mFieldDesc) << "'";
			}
			else if (mFieldDesc->type() == FieldDescriptor::Type::TYPE_INT64)
			{
				sqlStream << pReflection->GetString(*this->mData, mFieldDesc);
			}
			else if (mFieldDesc->type() == FieldDescriptor::Type::TYPE_INT32)
			{
				sqlStream << pReflection->GetString(*this->mData, mFieldDesc);
			}
			else
			{
				SayNoDebugFatal("delete key must int64 or int32 or string");
			}
			this->mSqlCommand = sqlStream.str();
		}
		return this->mSqlCommand;
	}
}