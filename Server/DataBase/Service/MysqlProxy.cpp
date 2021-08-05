#include "MysqlProxy.h"
#include <Coroutine/CoroutineManager.h>
#include <Manager/MysqlManager.h>
#include <Manager/ProtocolManager.h>
#include <Manager/ThreadTaskManager.h>
#include <MysqlClient/MysqlThreadTask.h>
#include <Protocol/db.pb.h>
namespace Sentry
{
    MysqlProxy::MysqlProxy()
    {
        this->mCorManager = nullptr;
        this->mMysqlManager = nullptr;
    }

    bool MysqlProxy::OnInit()
    {
        SayNoAssertRetFalse_F(this->mMysqlManager = this->GetManager<MysqlManager>());
        SayNoAssertRetFalse_F(this->mCorManager = this->GetManager<CoroutineManager>());
        SayNoAssertRetFalse_F(this->mTaskManager = this->GetManager<ThreadTaskManager>());
        SayNoAssertRetFalse_F(this->mProtocolManager = this->GetManager<ProtocolManager>());

        REGISTER_FUNCTION_2(MysqlProxy::Add, s2s::MysqlOper_Request, s2s::MysqlOper_Response);
        REGISTER_FUNCTION_2(MysqlProxy::Save, s2s::MysqlOper_Request, s2s::MysqlOper_Response);
        REGISTER_FUNCTION_2(MysqlProxy::Delete, s2s::MysqlOper_Request, s2s::MysqlOper_Response);
        REGISTER_FUNCTION_2(MysqlProxy::QueryData, s2s::MysqlQuery_Request, s2s::MysqlQuery_Response);

        return true;
    }

    void MysqlProxy::OnInitComplete()
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
#ifdef SOEASY_DEBUG
        SayNoDebugInfo(sql);
#endif
        const std::string &tab = this->mMysqlManager->GetDataBaseName();
        shared_ptr<MysqlThreadTask> mysqlTask = make_shared<MysqlThreadTask>(this->mMysqlManager, tab, sql);

        if (!this->mTaskManager->StartInvokeTask(mysqlTask))
        {
            return XCode::MysqlStartTaskFail;
        }

        this->mCorManager->YieldReturn();
        response.set_errorstr(mysqlTask->GetErrorStr());
#ifdef SOEASY_DEBUG
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
#ifdef SOEASY_DEBUG
        SayNoDebugInfo(sql);
#endif
        const std::string &tab = this->mMysqlManager->GetDataBaseName();
        shared_ptr<MysqlThreadTask> mysqlTask = make_shared<MysqlThreadTask>(this->mMysqlManager, tab, sql);

        if (!this->mTaskManager->StartInvokeTask(mysqlTask))
        {
            return XCode::MysqlStartTaskFail;
        }

        this->mCorManager->YieldReturn();
        response.set_errorstr(mysqlTask->GetErrorStr());
#ifdef SOEASY_DEBUG
        long long t = TimeHelper::GetMilTimestamp() - mysqlTask->GetStartTime();
        SayNoDebugWarning("save sql use time [" << t / 1000.0f << "s]");
#endif// SOEASY_DEBUG
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
#ifdef SOEASY_DEBUG
        SayNoDebugInfo(sql);
#endif
        const std::string &tab = this->mMysqlManager->GetDataBaseName();
        shared_ptr<MysqlThreadTask> mysqlTask = make_shared<MysqlThreadTask>(this->mMysqlManager, tab, sql);

        if (!this->mTaskManager->StartInvokeTask(mysqlTask))
        {
            return XCode::MysqlStartTaskFail;
        }
        this->mCorManager->YieldReturn();
        response.set_errorstr(mysqlTask->GetErrorStr());
#ifdef SOEASY_DEBUG
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
#ifdef SOEASY_DEBUG
        SayNoDebugInfo(sql);
#endif
        const std::string &tab = this->mMysqlManager->GetDataBaseName();
        shared_ptr<MysqlThreadTask> mysqlTask = make_shared<MysqlThreadTask>(this->mMysqlManager, tab, sql);

        if (!this->mTaskManager->StartInvokeTask(mysqlTask))
        {
            return XCode::MysqlStartTaskFail;
        }

        this->mCorManager->YieldReturn();
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


    // XCode MysqlProxy::Query(Message &requestData, Message &responseData)
    // {
    //     auto queryTask = this->mMysqlManager->CreateMysqlTask<MysqlQueryTask>();
    //     XCode startCode = this->mMysqlManager->StartTask(queryTask, &responseData);
    //     if (startCode != XCode::Successful)
    //     {
    //         responseData.Clear();
    //         return startCode;
    //     }
    //     this->mCorManager->YieldReturn();
    //     const std::vector<std::string> &queryDatas = queryTask->GetQueryDatas();
    //     if (queryDatas.empty())
    //     {
    //         return XCode::Failure;
    //     }

    //     const std::string &json = queryDatas[0];
    //     if (!util::JsonStringToMessage(json, &responseData).ok())
    //     {
    //         return XCode::JsonCastProtocbufFail;
    //     }
    //     return queryTask->GetErrorCode();
    // }

    // XCode MysqlProxy::Insert(Message &requestData)
    // {
    //     auto insertTask = this->mMysqlManager->CreateMysqlTask<MysqlInsertTask>();
    //     XCode startCode = this->mMysqlManager->StartTask(insertTask, &requestData);
    //     if (startCode != XCode::Successful)
    //     {
    //         return startCode;
    //     }
    //     this->mCorManager->YieldReturn();
    //     return insertTask->GetErrorCode();
    // }
    // XCode MysqlProxy::Update(Message &requestData)
    // {
    //     auto updateTask = this->mMysqlManager->CreateMysqlTask<MysqlUpdateTask>();
    //     XCode startCode = this->mMysqlManager->StartTask(updateTask, &requestData);
    //     if (startCode != XCode::Successful)
    //     {
    //         return startCode;
    //     }
    //     this->mCorManager->YieldReturn();
    //     return updateTask->GetErrorCode();
    // }
    // XCode MysqlProxy::Delete(Message &requestData)
    // {
    //     auto deleteTask = this->mMysqlManager->CreateMysqlTask<MysqlDeleteTask>();
    //     XCode startCode = this->mMysqlManager->StartTask(deleteTask, &requestData);
    //     if (startCode != XCode::Successful)
    //     {
    //         return startCode;
    //     }
    //     this->mCorManager->YieldReturn();
    //     return deleteTask->GetErrorCode();
    // }
}// namespace Sentry