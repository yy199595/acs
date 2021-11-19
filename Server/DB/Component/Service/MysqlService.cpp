#include "MysqlService.h"
#include <Scene/MysqlComponent.h>
#include <Scene/TaskPoolComponent.h>
#include <MysqlClient/MyqslTask.h>
#include <Core/App.h>
#include <Pool/MessagePool.h>
namespace GameKeeper
{
    MysqlService::MysqlService()
    {
        this->mCorComponent = nullptr;
        this->mMysqlManager = nullptr;
    }

    bool MysqlService::Awake()
    {
		this->mCorComponent = App::Get().GetCorComponent();
        GKAssertRetFalse_F(this->mMysqlManager = this->GetComponent<MysqlComponent>());
        GKAssertRetFalse_F(this->mTaskManager = this->GetComponent<TaskPoolComponent>());
		__add_method(MysqlService::Add);
		__add_method(MysqlService::Save);
		__add_method(MysqlService::Query);
		__add_method(MysqlService::Delete);
        __add_method(MysqlService::Invoke);
		
        return true;
    }

    void MysqlService::Start()
    {
        db::UserAccountData userData;
        userData.set_account("646585122@qq.com");
        userData.set_userid(420625199511045331);
    }

    XCode MysqlService::Add(const s2s::MysqlOper_Request &request, s2s::MysqlOper_Response &response)
    {
        if(!request.has_data())
        {
            return XCode::CallArgsError;
        }
		Message * message = MessagePool::NewByData(request.data());
        if (message == nullptr)
        {
			return XCode::ParseMessageError;
        }
        std::string sql;
        if (!this->mMysqlManager->GetAddSqlCommand(*message, sql))
        {
            return XCode::CallArgsError;
        }
        MyqslTask mysqlTask(this->mMysqlManager->GetDataBaseName(), sql);

        if (!this->mTaskManager->StartTask(&mysqlTask))
        {
            return XCode::MysqlStartTaskFail;
        }

        this->mCorComponent->YieldReturn();
        response.set_errorstr(mysqlTask.GetErrorStr());
        return mysqlTask.GetErrorCode();
    }

    XCode MysqlService::Save(const s2s::MysqlOper_Request &request, s2s::MysqlOper_Response &response)
    {
        if(!request.has_data())
        {
            return XCode::CallArgsError;
        }
		Message * message = MessagePool::New(request.data());
		if (message == nullptr)
		{
			return XCode::ParseMessageError;
		}
        
        std::string sql;
        if (!this->mMysqlManager->GetSaveSqlCommand(*message, sql))
        {
            return XCode::CallArgsError;
        }
		MyqslTask mysqlTask(this->mMysqlManager->GetDataBaseName(), sql);
        if (!this->mTaskManager->StartTask(&mysqlTask))
        {
            return XCode::MysqlStartTaskFail;
        }

        this->mCorComponent->YieldReturn();
        response.set_errorstr(mysqlTask.GetErrorStr());
        return mysqlTask.GetErrorCode();
    }

    XCode MysqlService::Delete(const s2s::MysqlOper_Request &request, s2s::MysqlOper_Response &response)
    {
		Message * message = MessagePool::New(request.data());
		if (message == nullptr)
		{
			return XCode::ParseMessageError;
		}
        std::string sql;
        if (!this->mMysqlManager->GetDeleteSqlCommand(*message, sql))
        {
            return XCode::CallArgsError;
        }
        MyqslTask mysqlTask(this->mMysqlManager->GetDataBaseName(), sql);

        if (!this->mTaskManager->StartTask(&mysqlTask))
        {
            return XCode::MysqlStartTaskFail;
        }
        this->mCorComponent->YieldReturn();
        response.set_errorstr(mysqlTask.GetErrorStr());
        return mysqlTask.GetErrorCode();
    }

    XCode MysqlService::Invoke(const s2s::MysqlAnyOper_Request &request, s2s::MysqlAnyOper_Response &response)
    {
        if(request.sql().empty())
        {
            return XCode::CallArgsError;
        }
        const SqlTableConfig * sqlTableConfig = this->mMysqlManager->GetTableConfig(request.tab());
        if(sqlTableConfig == nullptr)
        {
            return XCode::CallArgsError;
        }
        MyqslTask mysqlTask(this->mMysqlManager->GetDataBaseName(), request.sql());
        if (!this->mTaskManager->StartTask(&mysqlTask))
        {
            return XCode::MysqlStartTaskFail;
        }

        this->mCorComponent->YieldReturn();
        XCode code = mysqlTask.GetErrorCode();
        if (code == XCode::Successful && !mysqlTask.GetQueryDatas().empty())
        {
            const std::string & name = sqlTableConfig->mProtobufName;
            const std::vector<std::string> &jsonArray = mysqlTask.GetQueryDatas();
            for (const std::string &json : jsonArray)
            {
                Message * message = MessagePool::NewByJson(name, json);
                if(message == nullptr)
                {
                    return XCode::JsonCastProtocbufFail;
                }
                response.add_querydatas()->PackFrom(*message);
            }
            return XCode::Successful;
        }
        response.set_errotstr(mysqlTask.GetErrorStr());
        return code;
    }

    XCode MysqlService::Query(const s2s::MysqlQuery_Request &request, s2s::MysqlQuery_Response &response)
    {
        if(!request.has_data())
        {
            return XCode::CallArgsError;
        }
		Message * message = MessagePool::NewByData(request.data());
        if (message == nullptr)
        {
            return XCode::ParseMessageError;
        }

        std::string sql;
        if (!this->mMysqlManager->GetQuerySqlCommand(*message, sql))
        {
            return XCode::CallArgsError;
        }

        MyqslTask mysqlTask(this->mMysqlManager->GetDataBaseName(), sql);
        if (!this->mTaskManager->StartTask(&mysqlTask))
        {
            return XCode::MysqlStartTaskFail;
        }

        this->mCorComponent->YieldReturn();
        XCode code = mysqlTask.GetErrorCode();
        if (code == XCode::Successful)
        {
            const std::vector<std::string> &jsonArray = mysqlTask.GetQueryDatas();
			for (const std::string &json : jsonArray)
			{
				Message * message = MessagePool::NewByJson(request.data(), json);
				if (message == nullptr)
				{
					return XCode::JsonCastProtocbufFail;
				}			
				response.add_querydatas()->PackFrom(*message);
			}
            return XCode::Successful;
        }
        response.set_errotstr(mysqlTask.GetErrorStr());
        return code;
    }
}// namespace GameKeeper