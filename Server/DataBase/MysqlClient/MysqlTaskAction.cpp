#include "MysqlTaskAction.h"
#include<Manager/MysqlManager.h>

namespace DataBase
{
	MysqlTaskAction::MysqlTaskAction(MysqlManager * mgr, long long id, const std::string & db, const std::string & sql)
		: ThreadTaskAction(mgr, id)
	{
		this->ResetTask(mgr, id, db, sql);
	}

	void MysqlTaskAction::ResetTask(MysqlManager * mgr, long long id, const std::string & db, const std::string & sql)
	{
		this->mActionId = id;
		this->mSqlCommand = sql;
		this->mDataBaseName = db;
		this->mMysqlManager = mgr;
		this->mMysqlQueryData = nullptr;	
	}

	void MysqlTaskAction::InvokeInThreadPool(long long threadId)
	{
		mMysqlQueryData = std::make_shared<MysqlQueryData>();
		SayNoMysqlSocket * mysqlSocket = this->mMysqlManager->GetMysqlSocket(threadId);
		if (mysqlSocket == nullptr)
		{
			this->SetCode(MysqlSocketIsNull);
			return;
		}
		if (mysql_select_db(mysqlSocket, this->mDataBaseName.c_str()) != 0)
		{
			this->SetCode(MysqlSelectDbFailure);
			return;
		}
		const char * data = this->mSqlCommand.c_str();
		const size_t lenght = this->mSqlCommand.length();
		if (mysql_real_query(mysqlSocket, data, lenght) != 0)
		{
			this->SetCode(MysqlInvokeFailure);
			mMysqlQueryData->SetErrorMessage(mysql_error(mysqlSocket));
			return;
		}
		MysqlQueryResult * queryResult = mysql_store_result(mysqlSocket);
		if (queryResult != nullptr)
		{		
			unsigned long rowCount = mysql_num_rows(queryResult);
			unsigned int fieldCount = mysql_field_count(mysqlSocket);			
			for (unsigned int index = 0; index < fieldCount; index++)
			{
				MYSQL_FIELD * field = mysql_fetch_field(queryResult);
				mMysqlQueryData->AddFieldName(field->name, field->name_length, index);
			}

			for (unsigned long count = 0; count < rowCount; count++)
			{
				MYSQL_ROW row = mysql_fetch_row(queryResult);
				unsigned long * lengths = mysql_fetch_lengths(queryResult);
				for (unsigned int index = 0; index < fieldCount; index++)
				{
					if (lengths[index] > 0 && row[index] != nullptr)
					{
						mMysqlQueryData->AddFieldContent(count, row[index], lengths[index]);
					}
				}
			}
			mysql_free_result(queryResult);
		}
		this->SetCode(MysqlSuccessful);
	}

	bool MysqlTaskAction::SetCode(XMysqlCode code)
	{
		this->mMysqlQueryData->SetErrorCode(code);
		return code == XMysqlCode::MysqlSuccessful;
	}
	
}