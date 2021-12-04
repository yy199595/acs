//
// Created by zmhy0073 on 2021/11/29.
//

#ifndef GAMEKEEPER_MYSQLRPCTASK_H
#define GAMEKEEPER_MYSQLRPCTASK_H
#include<memory>
#include"XCode/XCode.h"
#include"Protocol/db.pb.h"
#include"Async/RpcTask/ProtoRpcTask.h"
using namespace google::protobuf;
namespace GameKeeper
{
    class MysqlRpcTask : public ProtoRpcTask
    {
    public:
        explicit MysqlRpcTask(XCode code);
        explicit MysqlRpcTask(int methodId, unsigned int rpcId);
        ~MysqlRpcTask() final;
    public:
        XCode AwakeGetCode();

        template<typename T>
        std::shared_ptr<T> AwaitGetData(size_t index = 0)
        {
            this->AsyncAwaitTask();
            if(index < 0 || index >= this->mResponseValues.size())
            {
                return nullptr;
            }
            return dynamic_pointer_cast<T>(this->mResponseValues[index]);
        }
        void OnResponse(const com::Rpc_Response *backData) final;
        size_t AwaitGetDataSize()
        {
            this->AsyncAwaitTask();
            return this->mResponseValues.size();
        }

    private:
        bool ParseOperResponse(const Any & any);
        bool ParseQueryResponse(const Any & any);
    private:
        std::vector<std::shared_ptr<Message>> mResponseValues;
    };
}


#endif //GAMEKEEPER_MYSQLRPCTASK_H
