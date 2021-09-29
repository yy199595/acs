#include "MysqlService.h"
#include <Coroutine/CoroutineComponent.h>
#include <Scene/MysqlComponent.h>
#include <Scene/ProtocolComponent.h>
#include <Scene/TaskComponent.h>
#include <MysqlClient/MyqslTask.h>
#include <Core/App.h>
#include <Util/TimeHelper.h>
#include <Pool/MessagePool.h>
namespace Sentry
{
    MysqlService::MysqlService()
    {
        this->mCorComponent = nullptr;
        this->mMysqlManager = nullptr;
    }

    bool MysqlService::Awake()
    {
		this->mCorComponent = App::Get().GetCoroutineComponent();
		SayNoAssertRetFalse_F(this->mTaskManager = Scene::GetComponent<TaskComponent>());
        SayNoAssertRetFalse_F(this->mMysqlManager = Scene::GetComponent<MysqlComponent>());
        SayNoAssertRetFalse_F(this->mProtocolManager = Scene::GetComponent<ProtocolComponent>());

		__add_method(MysqlService::Add);
		__add_method(MysqlService::Save);
		__add_method(MysqlService::Query);
		__add_method(MysqlService::Delete);
		
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
		const std::string & name = request.protocolname();
        const std::string & data = request.protocolmessage();
		
		Message * message = MessagePool::NewByData(name, data);
        if (message == nullptr)
        {
			return XCode::ParseMessageError;
        }
        std::string sql;
        if (!this->mMysqlManager->GetAddSqlCommand(*message, sql))
        {
            return XCode::CallArgsError;
        }
#ifdef _DEBUG
        SayNoDebugInfo(sql);
#endif
        const std::string &tab = this->mMysqlManager->GetDataBaseName();
        MyqslTask * mysqlTask = new MyqslTask(tab, sql);

        if (!this->mTaskManager->StartTask(mysqlTask))
        {
			delete mysqlTask;
            return XCode::MysqlStartTaskFail;
        }

        this->mCorComponent->YieldReturn();
        response.set_errorstr(mysqlTask->GetErrorStr());
#ifdef _DEBUG
        long long t = TimeHelper::GetMilTimestamp() - mysqlTask->GetStartTime();
        SayNoDebugWarning("add sql use time [" << t / 1000.0f << "s]");
#endif// SOEASY_DEBUG
        return mysqlTask->GetErrorCode();
    }

    XCode MysqlService::Save(const s2s::MysqlOper_Request &request, s2s::MysqlOper_Response &response)
    {
		const std::string & name = request.protocolname();
        const std::string & data = request.protocolmessage();
		Message * message = MessagePool::NewByData(name, data);
		if (message == nullptr)
		{
			return XCode::ParseMessageError;
		}
        
        std::string sql;
        if (!this->mMysqlManager->GetSaveSqlCommand(*message, sql))
        {
            return XCode::CallArgsError;
        }
#ifdef _DEBUG
        SayNoDebugInfo(sql);
#endif
        const std::string &tab = this->mMysqlManager->GetDataBaseName();
		MyqslTask * mysqlTask = new MyqslTask(tab, sql);

        if (!this->mTaskManager->StartTask(mysqlTask))
        {
            return XCode::MysqlStartTaskFail;
        }

        this->mCorComponent->YieldReturn();
        response.set_errorstr(mysqlTask->GetErrorStr());
#ifdef _DEBUG
        long long t = TimeHelper::GetMilTimestamp() - mysqlTask->GetStartTime();
        SayNoDebugWarning("save sql use time [" << t / 1000.0f << "s]");
#endif
        return mysqlTask->GetErrorCode();
    }

    XCode MysqlService::Delete(const s2s::MysqlOper_Request &request, s2s::MysqlOper_Response &response)
    {
		const std::string & name = request.protocolname();
        const std::string & data = request.protocolmessage();
		Message * message = MessagePool::NewByData(name, data);
		if (message == nullptr)
		{
			return XCode::ParseMessageError;
		}
        std::string sql;
        if (!this->mMysqlManager->GetDeleteSqlCommand(*message, sql))
        {
            return XCode::CallArgsError;
        }
#ifdef _DEBUG
        SayNoDebugInfo(sql);
#endif
        const std::string &tab = this->mMysqlManager->GetDataBaseName();
        MyqslTask * mysqlTask = new MyqslTask(tab, sql);

        if (!this->mTaskManager->StartTask(mysqlTask))
        {
			delete mysqlTask;
            return XCode::MysqlStartTaskFail;
        }
        this->mCorComponent->YieldReturn();
        response.set_errorstr(mysqlTask->GetErrorStr());
#ifdef _DEBUG
        long long t = TimeHelper::GetMilTimestamp() - mysqlTask->GetStartTime();
        SayNoDebugWarning("delete sql use time [" << t / 1000.0f << "s]");
#endif// SOEASY_DEBUG
        return mysqlTask->GetErrorCode();
    }

    XCode MysqlService::Query(const s2s::MysqlQuery_Request &request, s2s::MysqlQuery_Response &response)
    {
		const std::string & name = request.protocolname();
		const std::string & data = request.protocolmessage();
		Message * message = MessagePool::NewByData(name, data);
        if (message == nullptr)
        {
            response.set_errotstr("create " + request.protocolname() + " fail");
            return XCode::CreatePorotbufFail;
        }
       
        std::string sql;
        if (!this->mMysqlManager->GetQuerySqlCommand(*message, sql))
        {
            return XCode::CallArgsError;
        }
        const std::string &tab = this->mMysqlManager->GetDataBaseName();
        MyqslTask * mysqlTask = new MyqslTask(tab, sql);
        if (!this->mTaskManager->StartTask(mysqlTask))
        {
			delete mysqlTask;
            return XCode::MysqlStartTaskFail;
        }

        this->mCorComponent->YieldReturn();
        XCode code = mysqlTask->GetErrorCode();
        if (code == XCode::Successful)
        {	
            const std::vector<std::string> &jsonArray = mysqlTask->GetQueryDatas();
			for (const std::string &json : jsonArray)
			{
				Message * message = MessagePool::NewByJson(name, json);
				if (message == nullptr)
				{
					return XCode::JsonCastProtocbufFail;
				}
				std::string *jsonData = response.add_querydatas();
				jsonData->append(message->SerializeAsString());
			}
            return XCode::Successful;
        }
        response.set_errotstr(mysqlTask->GetErrorStr());
#ifdef SOEASY_DEBUG
        long long t = TimeHelper::GetMilTimestamp() - mysqlTask->GetStartTime();
        SayNoDebugWarning("query sql use time [" << t / 1000.0f << "s]");
#endif
        return code;
    }
}// namespace Sentry