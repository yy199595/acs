#include "MysqlQueryTask.h"
#include <Manager/MysqlManager.h>
#include <Coroutine/CoroutineManager.h>
#include <QueryResult/InvokeResultData.h>
namespace SoEasy
{
	MysqlQueryTask::MysqlQueryTask(MysqlManager *mgr, long long id, const std::string &db)
		: MysqlTaskBase(mgr, id, db)
	{
		this->mData = nullptr;
	}

	bool MysqlQueryTask::InitTask(const std::string tab, CoroutineManager *corMgr, Message *data)
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

	bool MysqlQueryTask::GetSqlCommand(std::string &sql)
	{
		if (!this->mSqlCommand.empty())
		{
			sql = this->mSqlCommand;
			return true;
		}

		std::stringstream sqlStream;
		const Descriptor *pDescriptor = this->mData->GetDescriptor();
		const Reflection *pReflection = this->mData->GetReflection();
		sqlStream << "select * from " << this->mTable << " where ";
		SqlTableConfig *tableConfig = this->GetTabConfig(this->mTable);

		for (size_t index = 0; index < tableConfig->mKeys.size(); index++)
		{
			const std::string &key = tableConfig->mKeys[index];
			const FieldDescriptor *fieldDesc = pDescriptor->FindFieldByName(key);
			sqlStream << key << "=";
			if(fieldDesc->type() == FieldDescriptor::Type::TYPE_STRING)
			{
				const std::string key = pReflection->GetString(*this->mData, fieldDesc);
				if(key == fieldDesc->default_value_string())
				{
					return false;
				}
				sqlStream << "'" << key << "'";
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
			else if(fieldDesc->type() == FieldDescriptor::Type::TYPE_UINT64)
			{
				unsigned long long key = pReflection->GetUInt64(*this->mData, fieldDesc);
				if(key == fieldDesc->default_value_uint64())
				{
					return false;
				}
				sqlStream << key;
			}
			else if(fieldDesc->type() == FieldDescriptor::TYPE_INT32)
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
			if (tableConfig->mKeys.size() > 1 && index < tableConfig->mKeys.size() - 1)
			{
				sqlStream << " and ";
			}
		}
		sql = this->mSqlCommand = sqlStream.str();
		return true;
	}

	/*void MysqlQueryTask::OnQueryFinish(QuertJsonWritre &jsonWriter)
	{
		MysqlTaskBase::OnQueryFinish(jsonWriter);
		const Reflection *pReflection = this->mData->GetReflection();
		const Descriptor *pDescriptor = this->mData->GetDescriptor();
		shared_ptr<rapidjson::Document> mDocument = make_shared<rapidjson::Document>();

		if (mDocument != nullptr && jsonWriter.Serialization(mDocument))
		{
			auto iter = mDocument->FindMember("data");
			if (iter != mDocument->MemberEnd())
			{
				rapidjson::Value &jsonValue = iter->value;
				SayNoAssertRet_F(jsonValue.IsObject());
				for (auto iter1 = jsonValue.MemberBegin(); iter1 != jsonValue.MemberEnd(); iter1++)
				{
					const std::string key = iter1->name.GetString();
					const FieldDescriptor *fieldDesc = pDescriptor->FindFieldByName(key);

					if (fieldDesc->type() == FieldDescriptor::TYPE_STRING || fieldDesc->type() == FieldDescriptor::TYPE_BYTES)
					{
						const char *str = iter1->value.GetString();
						const size_t size = iter1->value.GetStringLength();
						if (str != nullptr && size > 0)
						{
							pReflection->SetString(mData, fieldDesc, std::string(str, size));
						}
					}
					else if (fieldDesc->type() == FieldDescriptor::TYPE_INT32)
					{
						int value = iter1->value.GetInt();
						pReflection->SetInt32(mData, fieldDesc, value);
					}
					else if (fieldDesc->type() == FieldDescriptor::TYPE_INT64)
					{
						long long value = iter1->value.GetInt64();
						pReflection->SetInt64(mData, fieldDesc, value);
					}
					else if (fieldDesc->type() == FieldDescriptor::TYPE_FLOAT)
					{
						float value = iter1->value.GetFloat();
						pReflection->SetFloat(mData, fieldDesc, value);
					}
					else if (fieldDesc->type() == FieldDescriptor::TYPE_DOUBLE)
					{
						double value = iter1->value.GetDouble();
						pReflection->SetDouble(mData, fieldDesc, value);
					}
				}
			}
		}
	}*/
}