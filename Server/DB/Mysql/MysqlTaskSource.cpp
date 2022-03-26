#include"MysqlTaskSource.h"
#include"Object/App.h"
#include"Util/JsonHelper.h"
#include"Component/Mysql/MysqlComponent.h"
#include"Component/Coroutine/TaskComponent.h"
#include"Component/Scene/ThreadPoolComponent.h"
namespace Sentry
{
	MysqlTaskSource::MysqlTaskSource(MysqlComponent* component)
		: mMsqlComponent(component)
	{

	}

	XCode MysqlTaskSource::Await(const std::string& sql)
	{
		this->mSqlCommand = std::move(sql);
		auto threadComponent = App::Get().GetComponent<ThreadPoolComponent>();
		if (!threadComponent->StartTask(this))
		{
			return XCode::MysqlStartTaskFail;
		}
		XCode code = this->mTaskSource.Await();
		if (code != XCode::Successful)
		{
#ifdef __DEBUG__
			LOG_ERROR(sql);
#endif
			return code;
		}
#ifdef __DEBUG__
		LOG_INFO(sql);
#endif
		return XCode::Successful;
	}

	bool MysqlTaskSource::Run()
	{
		MysqlClient* mysqlSocket = this->mMsqlComponent->GetMysqlClient();
		if (mysqlSocket == nullptr)
		{
			this->mErrorString = "mysql socket is null";
			this->mTaskSource.SetResult(XCode::MysqlSocketIsNull);
			return true;
		}

		if (mysql_real_query(mysqlSocket, mSqlCommand.c_str(), mSqlCommand.size()) != 0)
		{
			this->mErrorString = mysql_error(mysqlSocket);
			this->mTaskSource.SetResult(XCode::MysqlInvokeFailure);
			return true;
		}
		MysqlQueryResult* queryResult = mysql_store_result(mysqlSocket);
		if (queryResult != nullptr)
		{
			std::string json;
			std::vector<MYSQL_FIELD*> fieldNameVector;
			unsigned long rowCount = mysql_num_rows(queryResult);
			unsigned int fieldCount = mysql_field_count(mysqlSocket);
			for (unsigned int index = 0; index < fieldCount; index++)
			{
				MYSQL_FIELD* field = mysql_fetch_field(queryResult);
				fieldNameVector.push_back(field);
			}
			if (rowCount == 1)
			{
				RapidJsonWriter jsonWrite;
				MYSQL_ROW row = mysql_fetch_row(queryResult);
				unsigned long* lengths = mysql_fetch_lengths(queryResult);
				for (size_t index = 0; index < fieldNameVector.size(); index++)
				{
					MYSQL_FIELD* field = fieldNameVector[index];
					this->WriteValue(jsonWrite, field, row[index], (int)lengths[index]);
				}
				if (jsonWrite.WriterToStream(json))
				{
					this->mQueryDatas.push(json);
				}
			}
			else
			{
				for (unsigned long count = 0; count < rowCount; count++)
				{
					RapidJsonWriter jsonWrite;
					MYSQL_ROW row = mysql_fetch_row(queryResult);
					unsigned long* lengths = mysql_fetch_lengths(queryResult);
					for (size_t index = 0; index < fieldNameVector.size(); index++)
					{
						MYSQL_FIELD* field = fieldNameVector[index];
						this->WriteValue(jsonWrite, field, row[index], (int)lengths[index]);
					}
					if (jsonWrite.WriterToStream(json))
					{
						this->mQueryDatas.push(json);
					}
				}
			}
			mysql_free_result(queryResult);
		}
		this->mTaskSource.SetResult(XCode::Successful);
		return true;
	}

	bool MysqlTaskSource::GetQueryData(std::string& data)
	{
		if (this->mQueryDatas.empty())
		{
			return false;
		}
		data.clear();
		data = this->mQueryDatas.front();
		this->mQueryDatas.pop();
		return true;
	}

	void MysqlTaskSource::WriteValue(RapidJsonWriter& jsonWriter, MYSQL_FIELD* field, const char* data, long size)
	{
		switch (field->type)
		{
		case enum_field_types::MYSQL_TYPE_LONG:
		case enum_field_types::MYSQL_TYPE_LONGLONG:
			this->mValue2 = std::atoll(data);
			if (this->mValue2 != 0)
			{
				jsonWriter.Add(field->name, this->mValue2);
			}
			break;
		case enum_field_types::MYSQL_TYPE_FLOAT:
		case enum_field_types::MYSQL_TYPE_DOUBLE:
			this->mValue1 = std::atof(data);
			if (this->mValue1 != 0)
			{
				jsonWriter.Add(field->name, this->mValue1);
			}
		default:
			if (data != nullptr && size > 0)
			{
				jsonWriter.Add(field->name, data, size);
			}
			break;
		}
	}
}// namespace Sentry