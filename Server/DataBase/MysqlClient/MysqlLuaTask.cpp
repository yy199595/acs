#include"MysqlLuaTask.h"
#include<Util/NumberHelper.h>
#include<Manager/MysqlManager.h>
namespace SoEasy
{
	MysqlLuaTask::MysqlLuaTask(MysqlManager * mgr, long long taskId, 
		lua_State * lua, int ref,const std::string & db, const std::string & sql)
		:ThreadTaskAction(mgr, taskId)
	{
		this->mLuaEnv = lua;
		this->mCroutineRef = ref;
		this->mCommandSql = sql;
		this->mDataBaseName = db;
		this->mMysqlManager = mgr;
	}

	void MysqlLuaTask::OnTaskFinish()
	{
		lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, this->mCroutineRef);
		if (lua_isthread(this->mLuaEnv, -1))
		{
			lua_State * coroutine = lua_tothread(this->mLuaEnv, -1);
			if (lua_getfunction(this->mLuaEnv, "JsonUtil", "ToObject"))
			{
				const char * json = this->mQueryJsonData.c_str();
				const size_t size = this->mQueryJsonData.size();
				lua_pushlstring(this->mLuaEnv, json, size);
				if (lua_pcall(this->mLuaEnv, 1, 1, 0) != 0)
				{
					SayNoDebugError("[lua error] " << lua_tostring(mLuaEnv, -1));
				}
			}
			lua_resume(coroutine, this->mLuaEnv, 1);
		}
		if (this->mErrorCode != XCode::Successful)
		{
			SayNoDebugError("[mysql error ]" << this->mErrorStr);
			SayNoDebugError("[mysql sql   ]" << this->mCommandSql);
		}
	}

	void MysqlLuaTask::InvokeInThreadPool(long long threadId)
	{
		QuertJsonWritre jsonWrite;
		SayNoMysqlSocket * mysqlSocket = this->mMysqlManager->GetMysqlSocket(threadId);
		if (mysqlSocket == nullptr)
		{
			this->mErrorCode = MysqlSocketIsNull;
			this->mErrorStr = "mysql socket null";
			this->EndWriteJson(jsonWrite);
			return;
		}

		if (mysql_select_db(mysqlSocket, this->mDataBaseName.c_str()) != 0)
		{
			this->mErrorCode = MysqlSelectDbFailure;
			this->mErrorStr = "select " + this->mDataBaseName + " fail";
			this->EndWriteJson(jsonWrite);
			return;
		}
		const char * data = this->mCommandSql.c_str();
		const size_t lenght = this->mCommandSql.length();
		if (mysql_real_query(mysqlSocket, data, lenght) != 0)
		{
			this->mErrorCode = MysqlInvokeFailure;
			this->mErrorStr = mysql_error(mysqlSocket);
			this->EndWriteJson(jsonWrite);
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
		}
		this->EndWriteJson(jsonWrite);
	}

	bool MysqlLuaTask::Start(lua_State * lua, int index, const std::string & db, const std::string & sql)
	{
		if (!lua_isthread(lua, index))
		{
			return false;
		}
		Applocation * app = Applocation::Get();
		lua_State * coroutine = lua_tothread(lua, index);
		int ref = luaL_ref(lua, LUA_REGISTRYINDEX);
		MysqlManager * mysqlManager = app->GetManager<MysqlManager>();
		if (mysqlManager == nullptr)
		{
			return false;
		}
		long long taskId = NumberHelper::Create();
		shared_ptr<MysqlLuaTask> taskAction =  std::make_shared<MysqlLuaTask>(mysqlManager, taskId, lua, ref, db, sql);
		return mysqlManager->StartTaskAction(taskAction);
	}

	void MysqlLuaTask::EndWriteJson(QuertJsonWritre & jsonWrite)
	{	
		jsonWrite.Write("code", (long long)this->mErrorCode);
		jsonWrite.Write("error", this->mErrorStr.c_str(), this->mErrorStr.size());
		if (!jsonWrite.Serialization(mQueryJsonData))
		{
			this->mErrorCode = XCode::RedisJsonParseFail;
			SayNoDebugFatal("mysql result cast json failure");
		}
	}

}
