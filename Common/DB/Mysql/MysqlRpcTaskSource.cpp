//
// Created by zmhy0073 on 2021/11/29.
//

#include"MysqlRpcTaskSource.h"
#include"Pool/MessagePool.h"
#include"Component/Rpc/RpcHandlerComponent.h"
namespace Sentry
{
    XCode  MysqlRpcTaskSource::GetCode()
    {
        this->mTaskSource.Await();
        return this->mCode;
    }

    std::shared_ptr<s2s::Mysql::Response> MysqlRpcTaskSource::GetResponse()
    {
        return this->mTaskSource.Await();
    }

    void MysqlRpcTaskSource::OnResponse(std::shared_ptr<com::Rpc_Response> response)
    {
        if (response == nullptr)
        {
            this->mCode = XCode::CallTimeout;
            this->mTaskSource.SetResult(nullptr);
            return;
        }
        this->mCode = (XCode) response->code();
        std::shared_ptr<s2s::Mysql::Response> data;
        if (this->mCode == XCode::Successful && response->has_data())
        {
            if (response->data().Is<s2s::Mysql::Response>())
            {
                data = std::shared_ptr<s2s::Mysql::Response>(new s2s::Mysql::Response());
                if (!response->data().UnpackTo(data.get()))
                {
                    std::move(data);
                    LOG_ERROR("parse mysql response error");
                }
            }
        }
        this->mTaskSource.SetResult(data);
    }
}