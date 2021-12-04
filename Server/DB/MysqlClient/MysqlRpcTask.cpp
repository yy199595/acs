//
// Created by zmhy0073 on 2021/11/29.
//

#include"MysqlRpcTask.h"
#include"Core/App.h"
#include"Pool/MessagePool.h"
#include"Scene/RpcConfigComponent.h"
namespace GameKeeper
{
    MysqlRpcTask::MysqlRpcTask(XCode code)
        : ProtoRpcTask(code)
    {

    }

    MysqlRpcTask::MysqlRpcTask(int methodId, unsigned int rpcId)
            : ProtoRpcTask(methodId, rpcId)
    {

    }

    MysqlRpcTask::~MysqlRpcTask() noexcept
    {
        this->mResponseValues.clear();
    }

    void MysqlRpcTask::OnResponse(const com::Rpc_Response *backData)
    {
        if(backData == nullptr)
        {
            this->mTaskState = TaskTimeout;
            this->mCode = XCode::CallTimeout;
#ifdef __DEBUG__
            int methodId = this->GetMethodId();
            auto configComponent = App::Get().GetComponent<RpcConfigComponent>();
            const ProtocolConfig * config = configComponent->GetProtocolConfig(methodId);
            LOG_ERROR(config->Service << "." << config->Method << " call time out");
#endif // __DEBUG__
        }
        else if(this->mTaskState == TaskAwait)
        {
            this->mTaskState = TaskFinish;
            this->mCode = (XCode)backData->code();
            if(this->mCode == XCode::Successful && backData->has_data())
            {
                if(!this->ParseOperResponse(backData->data()))
                {
                    this->ParseQueryResponse(backData->data());
                }
            }
        }
        this->RestoreAsyncTask();
    }

    bool MysqlRpcTask::ParseOperResponse(const google::protobuf::Any &any)
    {
        s2s::MysqlAnyOper_Response response;
        if (!any.Is<s2s::MysqlAnyOper_Response>() || !any.UnpackTo(&response))
        {
            return false;
        }
        for (int index = 0; index < response.querydatas_size(); index++)
        {
            const Any &data = response.querydatas(index);
            Message *message = MessagePool::NewByData(data, true);
            if (message != nullptr)
            {
                std::shared_ptr<Message> data(message);
                this->mResponseValues.emplace_back(data);
            }
        }
        return true;
    }

    bool MysqlRpcTask::ParseQueryResponse(const google::protobuf::Any &any)
    {
        s2s::MysqlQuery_Response response;
        if (!any.Is<s2s::MysqlQuery_Response>() || !any.UnpackTo(&response))
        {
            return false;
        }
        for (int index = 0; index < response.querydatas_size(); index++)
        {
            const Any &data = response.querydatas(index);
            Message *message = MessagePool::NewByData(data, true);
            if (message != nullptr)
            {
                std::shared_ptr<Message> data(message);
                this->mResponseValues.emplace_back(data);
            }
        }
        return true;
    }

    XCode MysqlRpcTask::AwakeGetCode()
    {
        this->AsyncAwaitTask();
        return this->mCode;
    }
}