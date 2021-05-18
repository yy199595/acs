#include"MysqlTaskBase.h"
#include<Manager/MysqlManager.h>
#include<Coroutine/CoroutineManager.h>
#include<QueryResult/InvokeResultData.h>
namespace SoEasy
{
	MysqlTaskBase::MysqlTaskBase(MysqlManager * mgr, long long id, const std::string & db, const std::string & sql)
		: ThreadTaskAction(mgr, id)
	{
		this->mSqlCommand = sql;
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
			jsonWrite.Write("code", this->mErrorCode);
			jsonWrite.Write("error", this->mErrorString);
			return;
		}
		
		if (mysql_select_db(mysqlSocket, this->mDataBaseName.c_str()) != 0)
		{
			this->mErrorCode = MysqlSelectDbFailure;
			this->mErrorString = "select " + this->mDataBaseName + " fail";
			jsonWrite.Write("code", this->mErrorCode);
			jsonWrite.Write("error", this->mErrorString);
			return;
		}
		const char * data = this->mSqlCommand.c_str();
		const size_t lenght = this->mSqlCommand.length();
		if (mysql_real_query(mysqlSocket, data, lenght) != 0)
		{
			this->mErrorCode = MysqlInvokeFailure;
			this->mErrorString = mysql_error(mysqlSocket);
			jsonWrite.Write("code", this->mErrorCode);
			jsonWrite.Write("error", this->mErrorString);
			return;
		}
		this->mErrorCode = XCode::Successful;
		MysqlQueryResult * queryResult = mysql_store_result(mysqlSocket);
		if (queryResult != nullptr)
		{		
			
			std::vector<char *> fieldNameVector;
			unsigned long rowCount = mysql_num_rows(queryResult);
			unsigned int fieldCount = mysql_field_count(mysqlSocket);
			for (unsigned int index = 0; index < fieldCount; index++)
			{
				MYSQL_FIELD * field = mysql_fetch_field(queryResult);
				fieldNameVector.push_back(field->name);
			}
			if (rowCount == 1)
			{			
				jsonWrite.StartWriteObject("data");
				MYSQL_ROW row = mysql_fetch_row(queryResult);
				unsigned long * lengths = mysql_fetch_lengths(queryResult);
				for (size_t index = 0; index < fieldNameVector.size(); index++)
				{
					const char * key = fieldNameVector[index];
					jsonWrite.Write(key, row[index], (int)lengths[index]);
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
						const char * key = fieldNameVector[index];
						jsonWrite.Write(key, row[index], (int)lengths[index]);
					}				
					jsonWrite.EndWriteObject();
				}
				jsonWrite.EndWriteArray();
			}	
			mysql_free_result(queryResult);
			jsonWrite.Write("code", this->mErrorCode);
			jsonWrite.Write("error", this->mErrorString);
			this->OnQueryFinish(jsonWrite);
		}
	}
}