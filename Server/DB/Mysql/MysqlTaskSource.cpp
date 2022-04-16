#include"MysqlTaskSource.h"
#include"Json/JsonWriter.h"
#include"MysqlHelper.h"
namespace Sentry
{
	MysqlTaskSource::MysqlTaskSource(const std::string & sql, s2s::Mysql::Response & response)
		: mSqlCommand(sql), mResponse(response)
	{

	}

	XCode MysqlTaskSource::Await()
	{
		return this->mTaskSource.Await();
	}

	void MysqlTaskSource::Run(MysqlSocket* mysqlSocket)
	{
		if (mysql_real_query(mysqlSocket, mSqlCommand.c_str(), mSqlCommand.size()) != 0)
		{
			this->mResponse.set_error(mysql_error(mysqlSocket));
			this->mTaskSource.SetResult(XCode::MysqlInvokeFailure);
			return;
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
				Json::Writer jsonWrite;
				MYSQL_ROW row = mysql_fetch_row(queryResult);
				unsigned long* lengths = mysql_fetch_lengths(queryResult);
				for (size_t index = 0; index < fieldNameVector.size(); index++)
				{
					MYSQL_FIELD* field = fieldNameVector[index];
					this->WriteValue(jsonWrite, field, row[index], (int)lengths[index]);
				}
				this->mResponse.add_json_array(jsonWrite.ToJsonString());
			}
			else
			{
				for (unsigned long count = 0; count < rowCount; count++)
				{
					Json::Writer jsonWrite;
					MYSQL_ROW row = mysql_fetch_row(queryResult);
					unsigned long* lengths = mysql_fetch_lengths(queryResult);
					for (size_t index = 0; index < fieldNameVector.size(); index++)
					{
						MYSQL_FIELD* field = fieldNameVector[index];
						this->WriteValue(jsonWrite, field, row[index], (int)lengths[index]);
					}
					this->mResponse.add_json_array(jsonWrite.ToJsonString());
				}
			}
			mysql_free_result(queryResult);
		}
		this->mTaskSource.SetResult(XCode::Successful);
	}

	void MysqlTaskSource::WriteValue(Json::Writer& jsonWriter, MYSQL_FIELD* field, const char* data, long size)
	{
		switch (field->type)
		{
		case enum_field_types::MYSQL_TYPE_LONG:
		case enum_field_types::MYSQL_TYPE_LONGLONG:
		{
			long long value1 = std::atoll(data);
			jsonWriter.AddMember(field->name, value1);
		}
			break;
		case enum_field_types::MYSQL_TYPE_FLOAT:
		case enum_field_types::MYSQL_TYPE_DOUBLE:
		{
			double value2 = std::atof(data);
			jsonWriter.AddMember(field->name, value2);
		}
		default:
			if (data != nullptr && size > 0)
			{
				jsonWriter.AddMember(field->name, data, size);
			}
			break;
		}
	}
}// namespace Sentry