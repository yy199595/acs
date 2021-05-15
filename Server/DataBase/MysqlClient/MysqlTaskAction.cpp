#include"MysqlTaskAction.h"
#include<Manager/MysqlManager.h>
#include<Coroutine/CoroutineManager.h>
#include<QueryResult/InvokeResultData.h>
namespace SoEasy
{
	MysqlTaskAction::MysqlTaskAction(MysqlManager * mgr, long long id, CoroutineManager * corMgr, const std::string & db, const std::string & sql)
		: ThreadTaskAction(mgr, id)
	{
		this->mSqlCommand = sql;
		this->mDataBaseName = db;
		this->mMysqlManager = mgr;
		this->mCoroutineMgr = corMgr;
		this->mCoroutineId = corMgr->GetCurrentCorId();
	}

	void MysqlTaskAction::OnTaskFinish()
	{
		if (this->mCoroutineMgr != nullptr)
		{
			this->mCoroutineMgr->Resume(this->mCoroutineId);
		}
	}

	void MysqlTaskAction::InvokeInThreadPool(long long threadId)
	{
		SayNoMysqlSocket * mysqlSocket = this->mMysqlManager->GetMysqlSocket(threadId);
		if (mysqlSocket == nullptr)
		{
			this->mErrorCode = MysqlSocketIsNull;
			return;
		}
		
		if (mysql_select_db(mysqlSocket, this->mDataBaseName.c_str()) != 0)
		{
			this->mErrorCode = MysqlSelectDbFailure;
			return;
		}
		const char * data = this->mSqlCommand.c_str();
		const size_t lenght = this->mSqlCommand.length();
		if (mysql_real_query(mysqlSocket, data, lenght) != 0)
		{
			this->mErrorCode = MysqlInvokeFailure;
			this->mErrorString = mysql_error(mysqlSocket);
			return;
		}
		this->mErrorCode = XCode::Successful;
		MysqlQueryResult * queryResult = mysql_store_result(mysqlSocket);
		if (queryResult != nullptr)
		{		
			QuertJsonWritre jsonWrite;
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
			if (!jsonWrite.Serialization(this->mDocument))
			{
				this->mErrorCode = XCode::RedisJsonParseFail;
				this->mErrorString = "mysql result cast json failure";
			}
		}
	}
	std::shared_ptr<InvokeResultData> MysqlTaskAction::GetInvokeData()
	{
		if (this->mErrorCode != XCode::Successful)
		{
			SayNoDebugError("[mysql error] " << this->mErrorString);
		}
		return std::make_shared<InvokeResultData>(mErrorCode, mErrorString, mDocument);
	}
}