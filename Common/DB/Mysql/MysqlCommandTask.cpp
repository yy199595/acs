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
			MysqlQueryResult  * queryResult = this->InvokeQuery(this->mCommand, this->mError);
			if(queryResult == nullptr)
			{
				return XCode::MysqlInvokeFailure;
			}
			unsigned long rowCount = mysql_num_rows(queryResult);
			unsigned int fieldCount = mysql_field_count(this->GetMysqlSocket());
			std::unique_ptr<MYSQL_FIELD *[]> fieldNameVector(new MYSQL_FIELD*[fieldCount]);
			for (unsigned int index = 0; index < fieldCount; index++)
			{
				fieldNameVector[index] = mysql_fetch_field(queryResult);
			}
			for (unsigned long count = 0; count < rowCount; count++)
			{
				MYSQL_ROW row = mysql_fetch_row(queryResult);
				unsigned long* lengths = mysql_fetch_lengths(queryResult);
				for (size_t index = 0; index < fieldCount; index++)
				{
					this->mTempMessage.Clear();
					MYSQL_FIELD* field = fieldNameVector[index];
					if(this->WriteValue(field, row[index], (int)lengths[index]))
					{
						Any * any = this->mResponse.add_datas();
						any->PackFrom(this->mTempMessage);
					}
				}
			}
			return XCode::Successful;
		}

		bool MysqlQueryCommandTask::WriteValue(MYSQL_FIELD* field, const char* data, long size)
		{
			const Descriptor * descriptor = this->mTempMessage.GetDescriptor();
			const FieldDescriptor * fieldDescriptor = descriptor->FindFieldByName(field->name);
			if(fieldDescriptor == nullptr)
			{
				return false;
			}
			const Reflection * reflection = this->mTempMessage.GetReflection();
			switch (field->type)
			{
			case enum_field_types::MYSQL_TYPE_LONG:
				if (fieldDescriptor->cpp_type() == FieldDescriptor::CPPTYPE_INT32)
				{
					int number = std::atoi(data);
					reflection->SetInt32(&this->mTempMessage, fieldDescriptor, number);
					return true;
				}
				if (fieldDescriptor->cpp_type() == FieldDescriptor::CPPTYPE_UINT32)
				{
					unsigned int number = (unsigned int)std::atol(data);
					reflection->SetUInt32(&this->mTempMessage, fieldDescriptor, number);
					return true;
				}
				return false;
			case enum_field_types::MYSQL_TYPE_LONGLONG:
				if (fieldDescriptor->cpp_type() == FieldDescriptor::CPPTYPE_INT64)
				{
					long long number = std::atoll(data);
					reflection->SetInt64(&this->mTempMessage, fieldDescriptor, number);
					return true;
				}
				if (fieldDescriptor->cpp_type() == FieldDescriptor::CPPTYPE_UINT64)
				{
					unsigned long long number = (unsigned long long)std::atoll(data);
					reflection->SetUInt64(&this->mTempMessage, fieldDescriptor, number);
					return true;
				}
				return false;
			case enum_field_types::MYSQL_TYPE_FLOAT:
				if (fieldDescriptor->cpp_type() == FieldDescriptor::CPPTYPE_FLOAT)
				{
					float number = (float)std::atof(data);
					reflection->SetFloat(&this->mTempMessage, fieldDescriptor, number);
					return true;
				}
				return false;
			case enum_field_types::MYSQL_TYPE_DOUBLE:
				if (fieldDescriptor->cpp_type() == FieldDescriptor::CPPTYPE_DOUBLE)
				{
					double number = std::atof(data);
					reflection->SetDouble(&this->mTempMessage, fieldDescriptor, number);
					return true;
				}
				return false;
			case enum_field_types::MYSQL_TYPE_STRING:
			case enum_field_types::MYSQL_TYPE_VAR_STRING:
				if (fieldDescriptor->cpp_type() == FieldDescriptor::CPPTYPE_STRING
					|| fieldDescriptor->cpp_type() == FieldDescriptor::TYPE_BYTES)
				{
					reflection->SetString(&this->mTempMessage,
						fieldDescriptor, std::string(data, size));
					return true;
				}
				else if(fieldDescriptor->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE)
				{

				}
				return false;
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