#include"MysqlQueryTask.h"
#include<Manager/MysqlManager.h>
#include<Coroutine/CoroutineManager.h>
#include<QueryResult/InvokeResultData.h>
namespace SoEasy
{
	MysqlQueryTask::MysqlQueryTask(MysqlManager * mgr, long long id, const std::string & db)
		:MysqlTaskBase(mgr, id, db)
	{

	}

	bool MysqlQueryTask::InitTask(const std::string tab, CoroutineManager * corMgr, shared_ptr<Message> data)
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
			const Descriptor * pDescriptor = this->mData->GetDescriptor();
			const Reflection * pReflection = this->mData->GetReflection();
			sqlStream << "select * from " << this->mTable << " where ";
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