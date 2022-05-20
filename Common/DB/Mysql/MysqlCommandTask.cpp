//
// Created by mac on 2022/5/20.
//
#include"MysqlHelper.h"
#include"MysqlCommandTask.h"
namespace Sentry
{
	namespace Mysql
	{
		XCode MysqlCommandTask::Init()
		{
			MysqlHelper mysqlHelper;
			if(!this->GetCommand(mysqlHelper, this->mCommand))
			{
				return XCode::CallArgsError;
			}
			return XCode::Successful;
		}
		XCode MysqlCommandTask::Await()
		{
			return this->mTaskSource.Await();
		}

		void MysqlCommandTask::Run(MysqlSocket* mysqlSocket)
		{
			const char * sql = this->mCommand.c_str();
			const size_t size = this->mCommand.size();
			if(mysql_real_query(mysqlSocket, sql, size) != 0)
			{
				this->mError.append(mysql_error(mysqlSocket));
				this->mTaskSource.SetResult(XCode::MysqlInvokeFailure);
				return;
			}
			MysqlQueryResult* queryResult = mysql_store_result(mysqlSocket);
			if (queryResult != nullptr)
			{
				this->OnComplete(mysqlSocket, queryResult);
				mysql_free_result(queryResult);
			}
			this->mTaskSource.SetResult(XCode::Successful);
		}
	}

	namespace Mysql
	{
		bool MysqlAddCommandTask::GetCommand(MysqlHelper& helper, std::string& sql)
		{
			return helper.ToSqlCommand(this->mRequest, sql);
		}
	}

	namespace Mysql
	{
		bool MysqlSaveCommandTask::GetCommand(MysqlHelper& helper, std::string& sql)
		{
			return helper.ToSqlCommand(this->mRequest, sql);
		}

	}

	namespace Mysql
	{
		bool MysqlQueryCommandTask::GetCommand(MysqlHelper& helper, std::string& sql)
		{
			return helper.ToSqlCommand(this->mRequest, sql);
		}

		void MysqlQueryCommandTask::OnComplete(MysqlSocket* mysqlSocket, MysqlQueryResult* queryResult)
		{
			std::vector<MYSQL_FIELD*> fieldNameVector;
			unsigned long rowCount = mysql_num_rows(queryResult);
			unsigned int fieldCount = mysql_field_count(mysqlSocket);
			for (unsigned int index = 0; index < fieldCount; index++)
			{
				MYSQL_FIELD* field = mysql_fetch_field(queryResult);
				fieldNameVector.push_back(field);
			}
			std::string json;
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
				if(jsonWrite.WriterStream(json))
				{
					this->mResponse.add_json_array(json);
				}
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
					if(jsonWrite.WriterStream(json))
					{
						this->mResponse.add_json_array(json);
					}
				}
			}
		}

		void MysqlQueryCommandTask::WriteValue(Json::Writer& jsonWriter,
				MYSQL_FIELD* field, const char* data, long size)
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
	}

	namespace Mysql
	{
		bool MysqlUpdateCommandTask::GetCommand(MysqlHelper& helper, std::string& sql)
		{
			return helper.ToSqlCommand(this->mRequest, sql);
		}
	}

	namespace Mysql
	{
		bool MysqlDeleteCommandTask::GetCommand(MysqlHelper& helper, std::string& sql)
		{
			return helper.ToSqlCommand(this->mRequest, sql);
		}
	}
}