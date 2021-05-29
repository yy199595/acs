#include"MysqlQueryTask.h"
#include<Coroutine/CoroutineManager.h>
#include<QueryResult/InvokeResultData.h>
namespace SoEasy
{
	MysqlQueryTask::MysqlQueryTask(MysqlManager * mgr, long long id, const std::string & db)
		:MysqlTaskBase(mgr, id, db)
	{

	}

	bool MysqlQueryTask::InitTask(const std::string tab, CoroutineManager * corMgr,const std::string key, shared_ptr<Message> data)
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

	void MysqlQueryTask::OnTaskFinish()
	{
		SayNoAssertRet_F(this->mCorManager);
		this->mCorManager->Resume(this->mCoroutineId);
	}

	const std::string & MysqlQueryTask::GetSqlCommand()
	{
		if (this->mSqlCommand.empty())
		{
			std::stringstream sqlStream;
			sqlStream << "select * from " << this->mTable << " where " << this->mKey << "=";
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
			this->mSqlCommand = sqlStream.str();
		}
		return this->mSqlCommand;
	}

	void MysqlQueryTask::OnQueryFinish(QuertJsonWritre & jsonWriter)
	{
		MysqlTaskBase::OnQueryFinish(jsonWriter);
		const Reflection * pReflection = this->mData->GetReflection();
		const Descriptor * pDescriptor = this->mData->GetDescriptor();
		shared_ptr<rapidjson::Document> mDocument = make_shared<rapidjson::Document>();
	
		if (mDocument != nullptr && jsonWriter.Serialization(mDocument))
		{
			auto iter = mDocument->FindMember("data");
			if (iter != mDocument->MemberEnd())
			{
				rapidjson::Value & jsonValue = iter->value;
				SayNoAssertRet_F(jsonValue.IsObject());
				for (auto iter1 = jsonValue.MemberBegin(); iter1 != jsonValue.MemberEnd(); iter1++)
				{
					const std::string key = iter1->name.GetString();
					const FieldDescriptor * fieldDesc = pDescriptor->FindFieldByName(key);
					
					if (fieldDesc->type() == FieldDescriptor::TYPE_STRING
						|| fieldDesc->type() == FieldDescriptor::TYPE_BYTES)
					{
						const char * str = iter1->value.GetString();
						const size_t size = iter1->value.GetStringLength();
						if (str != nullptr && size > 0)
						{
							pReflection->SetString(mData.get(), fieldDesc, std::string(str, size));
						}					
					}
					else if (fieldDesc->type() == FieldDescriptor::TYPE_INT32)
					{
						int value = iter1->value.GetInt();
						pReflection->SetInt32(mData.get(), fieldDesc,value);
					}					
					else if (fieldDesc->type() == FieldDescriptor::TYPE_INT64)
					{
						long long value = iter1->value.GetInt64();
						pReflection->SetInt64(mData.get(), fieldDesc, value);
					}
					else if (fieldDesc->type() == FieldDescriptor::TYPE_FLOAT)
					{
						float value = iter1->value.GetFloat();
						pReflection->SetFloat(mData.get(), fieldDesc, value);
					}
					else if (fieldDesc->type() == FieldDescriptor::TYPE_DOUBLE)
					{
						double value = iter1->value.GetDouble();
						pReflection->SetDouble(mData.get(), fieldDesc, value);
					}
				}
			}
		}
	}
}