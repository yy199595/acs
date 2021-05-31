#include"MysqlTaskBase.h"
#include<Manager/MysqlManager.h>
#include<Coroutine/CoroutineManager.h>
#include<QueryResult/InvokeResultData.h>
namespace SoEasy
{
	MysqlTaskBase::MysqlTaskBase(MysqlManager * mgr, long long id, const std::string & db)
		: ThreadTaskAction(mgr, id)
	{
		this->mDataBaseName = db;
		this->mMysqlManager = mgr;
	}

	void MysqlTaskBase::InvokeInThreadPool(long long threadId)
	{
		QuertJsonWritre jsonWrite;
		SayNoMysqlSocket * mysqlSocket = this->mMysqlManager->GetMysqlSocket(threadId);
		if (mysqlSocket == nullptr)
		{
			this->mErrorCode = MysqlSocketIsNull;
			this->mErrorString = "mysql socket is null";
			this->OnQueryFinish(jsonWrite);
			return;
		}
		
		if (mysql_select_db(mysqlSocket, this->mDataBaseName.c_str()) != 0)
		{
			this->mErrorCode = MysqlSelectDbFailure;
			this->mErrorString = "select " + this->mDataBaseName + " fail";
			this->OnQueryFinish(jsonWrite);
			return;
		}
		const std::string & sql = this->GetSqlCommand();
		if (mysql_real_query(mysqlSocket, sql.c_str(), sql.size()) != 0)
		{
			this->mErrorCode = MysqlInvokeFailure;
			this->mErrorString = mysql_error(mysqlSocket);
			this->OnQueryFinish(jsonWrite);
			return;
		}
		this->mErrorCode = XCode::Successful;
		MysqlQueryResult * queryResult = mysql_store_result(mysqlSocket);
		if (queryResult != nullptr)
		{		
			
			std::vector<MYSQL_FIELD *> fieldNameVector;
			unsigned long rowCount = mysql_num_rows(queryResult);
			unsigned int fieldCount = mysql_field_count(mysqlSocket);
			for (unsigned int index = 0; index < fieldCount; index++)
			{
				MYSQL_FIELD * field = mysql_fetch_field(queryResult);
				fieldNameVector.push_back(field);
			}
			if (rowCount == 1)
			{			
				jsonWrite.StartWriteObject("data");
				MYSQL_ROW row = mysql_fetch_row(queryResult);
				unsigned long * lengths = mysql_fetch_lengths(queryResult);
				for (size_t index = 0; index < fieldNameVector.size(); index++)
				{
					MYSQL_FIELD * field = fieldNameVector[index];
					this->WriteValue(jsonWrite, field, row[index], (int)lengths[index]);
				}
				jsonWrite.EndWriteObject();
			}
			else
			{
				jsonWrite.StartWriteArray("data");
				for (unsigned long count = 0; count < rowCount; count++)
				{
					jsonWrite.StartWriteObject();
					MYSQL_ROW row = mysql_fetch_row(queryResult);
					unsigned long * lengths = mysql_fetch_lengths(queryResult);
					for (size_t index = 0; index < fieldNameVector.size(); index++)
					{
						MYSQL_FIELD * field = fieldNameVector[index];
						this->WriteValue(jsonWrite, field, row[index], (int)lengths[index]);
					}				
					jsonWrite.EndWriteObject();
				}
				jsonWrite.EndWriteArray();
			}	
			mysql_free_result(queryResult);
		}
		this->OnQueryFinish(jsonWrite);
	}
	void MysqlTaskBase::OnQueryFinish(QuertJsonWritre & jsonWriter)
	{
		jsonWriter.Write("code", this->mErrorCode);
		jsonWriter.Write("error", this->mErrorString);
		if (this->mErrorCode != XCode::Successful)
		{
			SayNoDebugError("[mysql error] " << this->mErrorString);
			SayNoDebugError("[mysql command] " << this->GetSqlCommand());
		}	
	}

	void MysqlTaskBase::WriteValue(QuertJsonWritre & jsonWriter, MYSQL_FIELD * field, const char * data, long size)
	{
		switch (field->type)
		{
		case enum_field_types::MYSQL_TYPE_LONG:
		case enum_field_types::MYSQL_TYPE_LONGLONG:
			jsonWriter.Write(field->name, data == nullptr ? 0 : std::atoll(data));
			break;
		case enum_field_types::MYSQL_TYPE_FLOAT:
		case enum_field_types::MYSQL_TYPE_DOUBLE:
			jsonWriter.Write(field->name, data == nullptr ? 0 : std::atof(data));
		default:
			jsonWriter.Write(field->name, data, size);
			break;
		}
	}
}