//
// Created by zmhy0073 on 2021/11/29.
//

#include"MysqlRpcTaskSource.h"
#include"Pool/MessagePool.h"
#include"Rpc/RpcComponent.h"
namespace GameKeeper
{
    size_t MysqlRpcTaskSource::GetDataSize()
    {
        auto response = this->mTaskSource.Await();
        if(response == nullptr){
            return 0;
        }
        return response->datas_size();
    }

    void MysqlRpcTaskSource::OnResponse(const com::Rpc_Response * response)
    {
        this->mCode = XCode::CallTimeout;
        s2s::MysqlResponse * data = nullptr;
        if(response != nullptr)
        {
            this->mCode = (XCode)response->code();
            if(mCode == XCode::Successful && response->has_data())
            {
                if(response->data().Is<s2s::MysqlResponse>())
                {
                    data = new s2s::MysqlResponse();
                    response->data().UnpackTo(data);
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