#include "MysqlProxy.h"
#include <Manager/MysqlManager.h>
namespace SoEasy
{
    bool MysqlProxy::OnInit()
    {
        SayNoAssertRetFalse_F(this->mMysqlManager = this->GetManager<MysqlManager>());
        this->BindFunction("MysqlProxy.Insert", std::bind(&MysqlProxy::Insert, this, args1));
        this->BindFunction("MysqlProxy.Update", std::bind(&MysqlProxy::Update, this, args1));
        this->BindFunction("MysqlProxy.Delete", std::bind(&MysqlProxy::Delete, this, args1));
        this->BindFunction("MysqlProxy.Query", std::bind(&MysqlProxy::Query, this, args1, args2));
        return true;
    }

    XCode MysqlProxy::Query(shared_ptr<Message> requestData, shared_ptr<Message> responseData)
    {
        if (this->mMysqlManager->QueryData(responseData))
        {
            return XCode::Successful;
        }
        return XCode::Failure;
    }

    XCode MysqlProxy::Insert(shared_ptr<Message> requestData)
    {
        if (this->mMysqlManager->QueryData(requestData))
        {
            return XCode::Successful;
        }
        return XCode::Failure;
    }
    XCode MysqlProxy::Update(shared_ptr<Message> requestData)
    {
        if (this->mMysqlManager->QueryData(requestData))
        {
            return XCode::Successful;
        }
        return XCode::Failure;
    }
    XCode MysqlProxy::Delete(shared_ptr<Message> requestData)
    {
        if (this->mMysqlManager->QueryData(requestData))
        {
            return XCode::Successful;
        }
        return XCode::Failure;
    }
}