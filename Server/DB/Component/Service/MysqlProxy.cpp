#include "MysqlProxy.h"
#include <Coroutine/CoroutineComponent.h>
#include <Scene/SceneMysqlComponent.h>
#include <Scene/SceneProtocolComponent.h>
#include <Scene/SceneTaskComponent.h>
#include <MysqlClient/MyqslTask.h>
#include <Core/App.h>
#include <Util/TimeHelper.h>
#include<google/protobuf/util/json_util.h>
namespace Sentry
{
    MysqlProxy::MysqlProxy()
    {
        this->mCorComponent = nullptr;
        this->mMysqlManager = nullptr;
    }

    bool MysqlProxy::Awake()
    {
		this->mCorComponent = App::Get().GetCoroutineComponent();
		SayNoAssertRetFalse_F(this->mTaskManager = Scene::GetComponent<SceneTaskComponent>());
        SayNoAssertRetFalse_F(this->mMysqlManager = Scene::GetComponent<SceneMysqlComponent>());
        SayNoAssertRetFalse_F(this->mProtocolManager = Scene::GetComponent<SceneProtocolComponent>());

		__ADD_SERVICE_METHOD__(MysqlProxy::Add);
		__ADD_SERVICE_METHOD__(MysqlProxy::Save);
		__ADD_SERVICE_METHOD__(MysqlProxy::Delete);
		__ADD_SERVICE_METHOD__(MysqlProxy::QueryData);
        return true;
    }

    void MysqlProxy::Start()
    {
        db::UserAccountData userData;
        userData.set_account("646585122@qq.com");
        userData.set_userid(420625199511045331);
    }

    XCode MysqlProxy::Add(long long, const s2s::MysqlOper_Request &request, s2s::MysqlOper_Response &response)
    {
        const std::string &messageData = request.protocolmessage();
        Message *protocolMessage = this->mProtocolManager->CreateMessage(request.protocolname());
        if (protocolMessage == nullptr)
        {
            return XCode::Failure;
        }
        if (!protocolMessage->ParseFromString(messageData))
        {
            return XCode::ParseMessageError;
        }
        std::string sql;
        if (!this->mMysqlManager->GetAddSqlCommand(*protocolMessage, sql))
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

    XCode MysqlProxy::Save(long long, const s2s::MysqlOper_Request &request, s2s::MysqlOper_Response &response)
    {
        const std::string &messageData = request.protocolmessage();
        Message *protocolMessage = this->mProtocolManager->CreateMessage(request.protocolname());
        if (protocolMessage == nullptr)
        {
            return XCode::Failure;
        }
        if (!protocolMessage->ParseFromString(messageData))
        {
            return XCode::ParseMessageError;
        }
        std::string sql;
        if (!this->mMysqlManager->GetSaveSqlCommand(*protocolMessage, sql))
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

    XCode MysqlProxy::Delete(long long, const s2s::MysqlOper_Request &request, s2s::MysqlOper_Response &response)
    {
        const std::string &messageData = request.protocolmessage();
        Message *protocolMessage = this->mProtocolManager->CreateMessage(request.protocolname());
        if (protocolMessage == nullptr)
        {
            return XCode::Failure;
        }
        if (!protocolMessage->ParseFromString(messageData))
        {
            return XCode::ParseMessageError;
        }
        std::string sql;
        if (!this->mMysqlManager->GetDeleleSqlCommand(*protocolMessage, sql))
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

    XCode MysqlProxy::QueryData(long long, const s2s::MysqlQuery_Request &request, s2s::MysqlQuery_Response &response)
    {
        Message *protocolMessage = this->mProtocolManager->CreateMessage(request.protocolname());
        if (protocolMessage == nullptr)
        {
            response.set_errotstr("create " + request.protocolname() + " fail");
            return XCode::CreatePorotbufFail;
        }
        const std::string &messageData = request.protocolmessage();

        if (!protocolMessage->ParseFromString(messageData))
        {
            return XCode::ParseMessageError;
        }
        std::string sql;
        if (!this->mMysqlManager->GetQuerySqlCommand(*protocolMessage, sql))
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
                if (!util::JsonStringToMessage(json, protocolMessage).ok())
                {
                    return XCode::JsonCastProtocbufFail;
                }
                std::string *jsonData = response.add_querydatas();
                jsonData->append(protocolMessage->SerializeAsString());
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