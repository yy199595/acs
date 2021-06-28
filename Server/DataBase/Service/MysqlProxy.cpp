#include "MysqlProxy.h"
#include <Other/ServiceNode.h>
#include <Manager/MysqlManager.h>
#include <Coroutine/CoroutineManager.h>
#include <MysqlClient/MysqlInsertTask.h>
#include <MysqlClient/MysqlUpdateTask.h>
#include <MysqlClient/MysqlQueryTask.h>
#include <MysqlClient/MysqlDeleteTask.h>
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
        this->Bind("MysqlProxy.Insert", MysqlOperAction(std::bind(&MysqlProxy::Insert, this, args1)));
        this->Bind("MysqlProxy.Update", MysqlOperAction(std::bind(&MysqlProxy::Update, this, args1)));
        this->Bind("MysqlProxy.Delete", MysqlOperAction(std::bind(&MysqlProxy::Delete, this, args1)));
        this->Bind("MysqlProxy.Query", MysqlQueryAction(std::bind(&MysqlProxy::Query, this, args1, args2)));
        return true;
    }

    XCode MysqlProxy::Query(Message & requestData, Message & responseData)
    {
        auto insertTask = this->mMysqlManager->CreateMysqlTask<MysqlInsertTask>();
        XCode startCode = this->mMysqlManager->StartTask(insertTask, &responseData);
        if (startCode != XCode::Successful)
        {
            responseData.Clear();
            return startCode;
        }
        this->mCorManager->YieldReturn();
        return insertTask->GetErrorCode();
    }

    XCode MysqlProxy::Insert(Message & requestData)
    {
        auto insertTask = this->mMysqlManager->CreateMysqlTask<MysqlInsertTask>();
        XCode startCode = this->mMysqlManager->StartTask(insertTask, &requestData);
        if (startCode != XCode::Successful)
        {
            return startCode;
        }
        this->mCorManager->YieldReturn();
        return insertTask->GetErrorCode();
    }
    XCode MysqlProxy::Update(Message & requestData)
    {
        auto updateTask = this->mMysqlManager->CreateMysqlTask<MysqlUpdateTask>();
        XCode startCode = this->mMysqlManager->StartTask(updateTask, &requestData);
        if (startCode != XCode::Successful)
        {
            return startCode;
        }
        this->mCorManager->YieldReturn();
        return updateTask->GetErrorCode();
    }
    XCode MysqlProxy::Delete(Message & requestData)
    {
        auto deleteTask = this->mMysqlManager->CreateMysqlTask<MysqlDeleteTask>();
        XCode startCode = this->mMysqlManager->StartTask(deleteTask, &requestData);
        if (startCode != XCode::Successful)
        {
            return startCode;
        }
        this->mCorManager->YieldReturn();
        return deleteTask->GetErrorCode();
    }
}