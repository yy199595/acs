//
// Created by mac on 2022/5/20.
//
#include"MysqlHelper.h"
#include"MysqlCommandTask.h"
namespace Sentry
{
	namespace Mysql
	{
		XCode MysqlCommandTask::Await()
		{
			TimerComponent * timerComponent = App::Get()->GetTimerComponent();
			long long timerId = timerComponent->DelayCall(5.0f, [this]()
			{
				this->mTaskSource.SetResult(XCode::CallTimeout);
			});
			XCode code = this->mTaskSource.Await();
			timerComponent->CancelTimer(timerId);
			return code;
		}

		bool MysqlCommandTask::Invoke(const std::string& sql, std::string& error)
		{
			if(mysql_real_query(this->mMysqlSocket, sql.c_str(), sql.size()) != 0)
			{
				error.append(mysql_error(this->mMysqlSocket));
				return false;
			}
			return true;
		}

		MysqlQueryResult* MysqlCommandTask::InvokeQuery(const std::string& sql, std::string& eror)
		{
			if(mysql_real_query(this->mMysqlSocket, sql.c_str(), sql.size()) != 0)
			{
				eror.append(mysql_error(this->mMysqlSocket));
				return nullptr;
			}
			MysqlQueryResult * mysqlQueryResult = mysql_store_result(this->mMysqlSocket);
			if(mysqlQueryResult != nullptr)
			{
				this->mResults.push(mysqlQueryResult);
				return mysqlQueryResult;
			}
			return nullptr;
		}

		void MysqlCommandTask::Run(MysqlSocket* mysqlSocket)
		{
			this->mMysqlSocket = mysqlSocket;
			this->mTaskSource.SetResult(this->OnInvoke());

			while(!this->mResults.empty())
			{
				mysql_free_result(this->mResults.front());
				this->mResults.pop();
			}
		}
	}

	namespace Mysql
	{
		bool MysqlAddCommandTask::Init()
		{
			MysqlHelper helper;
			this->mError.clear();
			this->mCommand.clear();
			return helper.ToSqlCommand(this->mRequest, this->mCommand);
		}

		XCode MysqlAddCommandTask::OnInvoke()
		{
			if(this->Invoke(this->mCommand, this->mError))
			{
				CONSOLE_LOG_ERROR(this->mError);
				CONSOLE_LOG_ERROR(this->mCommand);
				return XCode::MysqlInvokeFailure;
			}
			return XCode::Successful;
		}
	}

	namespace Mysql
	{
		bool MysqlSaveCommandTask::Init()
		{
			MysqlHelper helper;
			this->mError.clear();
			this->mCommand.clear();
			return helper.ToSqlCommand(this->mRequest, this->mCommand);
		}

		XCode MysqlSaveCommandTask::OnInvoke()
		{
			if(this->Invoke(this->mCommand, this->mError))
			{
				CONSOLE_LOG_ERROR(this->mError);
				CONSOLE_LOG_ERROR(this->mCommand);
				return XCode::MysqlInvokeFailure;
			}
			return XCode::Successful;
		}
	}

	namespace Mysql
    {
        bool MysqlQueryCommandTask::Init()
        {
            MysqlHelper mysqlHelper;
            return mysqlHelper.ToSqlCommand(this->mRequest, this->mCommand);
        }

        XCode MysqlQueryCommandTask::OnInvoke()
        {
            MysqlQueryResult *queryResult = this->InvokeQuery(this->mCommand, this->mError);
            if (queryResult == nullptr)
            {
                return XCode::MysqlInvokeFailure;
            }
            unsigned long rowCount = mysql_num_rows(queryResult);
            unsigned int fieldCount = mysql_field_count(this->GetMysqlSocket());
            std::unique_ptr<MYSQL_FIELD *[]> fieldNameVector(new MYSQL_FIELD *[fieldCount]);
            for (unsigned int index = 0; index < fieldCount; index++)
            {
                fieldNameVector[index] = mysql_fetch_field(queryResult);
            }
            for (unsigned long count = 0; count < rowCount; count++)
            {
                Json::Writer jsonWriter;
                std::string *json = this->mResponse.add_jsons();
                MYSQL_ROW row = mysql_fetch_row(queryResult);
                unsigned long *lengths = mysql_fetch_lengths(queryResult);
                for (size_t index = 0; index < fieldCount; index++)
                {
                    MYSQL_FIELD *field = fieldNameVector[index];
                    if(row[index] != NULL && lengths[index] > 0)
                    {
                        this->WriteValue(jsonWriter, field, row[index], (int) lengths[index]);
                    }
                }
                jsonWriter.WriterStream(*json);
            }
            return XCode::Successful;
        }

        bool MysqlQueryCommandTask::WriteValue(Json::Writer &jsonWriter, MYSQL_FIELD *field, const char *data, long size)
        {
            switch (field->type)
            {
                case enum_field_types::MYSQL_TYPE_LONG:
                case enum_field_types::MYSQL_TYPE_LONGLONG:
                {
                    long long number = std::atoll(data);
					jsonWriter << field->name << number;
                }
                    return true;
                case enum_field_types::MYSQL_TYPE_FLOAT:
                case enum_field_types::MYSQL_TYPE_DOUBLE:
                {
                    double number = std::atof(data);
                    jsonWriter << field->name << number;
                }
                    return true;
                case enum_field_types::MYSQL_TYPE_STRING:
                case enum_field_types::MYSQL_TYPE_VAR_STRING:
                {
					jsonWriter << field->name;
                    jsonWriter.AddBinString(data, size);
                    return true;
                }
            }
            return false;
        }
    }

	namespace Mysql
	{
		bool MysqlUpdateCommandTask::Init()
		{
			MysqlHelper helper;
			return helper.ToSqlCommand(this->mRequest, this->mCommand);
		}

		XCode MysqlUpdateCommandTask::OnInvoke()
		{
			if(!this->Invoke(this->mCommand, this->mError))
			{
				CONSOLE_LOG_ERROR(this->mError);
				CONSOLE_LOG_ERROR(this->mCommand);
				return XCode::MysqlInvokeFailure;
			}
			return XCode::Successful;
		}
	}

	namespace Mysql
	{
		bool MysqlDeleteCommandTask::Init()
		{
			MysqlHelper helper;
			return helper.ToSqlCommand(this->mRequest, this->mCommand);
		}

		XCode MysqlDeleteCommandTask::OnInvoke()
		{
			if(!this->Invoke(this->mCommand, this->mError))
			{
				CONSOLE_LOG_ERROR(this->mError);
				CONSOLE_LOG_ERROR(this->mCommand);
				return XCode::MysqlInvokeFailure;
			}
			return XCode::Successful;
		}
	}
}