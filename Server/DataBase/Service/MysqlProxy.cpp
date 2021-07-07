#include "MysqlProxy.h"
#include <Other/ServiceNode.h>
#include <Manager/MysqlManager.h>
#include <Coroutine/CoroutineManager.h>
#include <MysqlClient/MysqlInsertTask.h>
#include <MysqlClient/MysqlUpdateTask.h>
#include <MysqlClient/MysqlQueryTask.h>
#include <MysqlClient/MysqlDeleteTask.h>
#include <Protocol/db.pb.h>
#include <google/protobuf/util/json_util.h>
#include <Pool/ProtocolPool.h>
namespace SoEasy
{
    MysqlProxy::MysqlProxy()
    {
        this->mMysqlNodeId = 0;
        this->mCorManager = nullptr;
        this->mMysqlManager = nullptr;
    }

    bool MysqlProxy::OnInit()
    {
        SayNoAssertRetFalse_F(this->mMysqlManager = this->GetManager<MysqlManager>());
        SayNoAssertRetFalse_F(this->mCorManager = this->GetManager<CoroutineManager>());

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

    XCode MysqlProxy::Add(long long, const s2s::MysqlOper_Request &requertData, s2s::MysqlOper_Response &response)
    {
        ProtocolPool *protocolPool = ProtocolPool::Get();
        Message *messageData = protocolPool->Create(requertData.protocolname());
        if (messageData == nullptr)
        {
            return XCode::Failure;
        }
        
    }

    XCode MysqlProxy::Save(long long, const s2s::MysqlOper_Request &requertData, s2s::MysqlOper_Response &response)
    {
    }

    XCode MysqlProxy::Delete(long long, const s2s::MysqlOper_Request &requertData, s2s::MysqlOper_Response &response)
    {
    }

    XCode MysqlProxy::QueryData(long long, const s2s::MysqlQuery_Response &requertData, s2s::MysqlQuery_Response &response)
    {
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
}