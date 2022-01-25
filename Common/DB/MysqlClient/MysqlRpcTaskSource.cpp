//
// Created by zmhy0073 on 2021/11/29.
//

#include"MysqlRpcTaskSource.h"
#include"Pool/MessagePool.h"
#include"Rpc/RpcComponent.h"
namespace Sentry
{
    size_t MysqlRpcTaskSource::GetDataSize()
    {
        auto response = this->mTaskSource.Await();
        if(response == nullptr){
            return 0;
        }
        return response->datas_size();
    }

    void MysqlRpcTaskSource::OnResponse(std::shared_ptr<com::Rpc_Response> response)
    {
        this->mCode = XCode::CallTimeout;
        std::shared_ptr<s2s::MysqlResponse> data;
        if(response != nullptr)
        {
            this->mCode = (XCode)response->code();
            if(mCode == XCode::Successful && response->has_data())
            {
                if(response->data().Is<s2s::MysqlResponse>())
                {
                    data = std::shared_ptr<s2s::MysqlResponse>(new s2s::MysqlResponse());
                    if(!response->data().UnpackTo(data.get()))
                    {
                        std::move(data);
                        LOG_ERROR("parse mysql response error");
                    }
                }
            }
        }
        this->mTaskSource.SetResult(data);
    }

    XCode MysqlRpcTaskSource::GetCode()
    {
        this->mTaskSource.Await();
        return this->mCode;
    }
}