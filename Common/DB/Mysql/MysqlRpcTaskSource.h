//
// Created by zmhy0073 on 2021/11/29.
//

#ifndef GAMEKEEPER_MYSQLRPCTASKSOURCE_H
#define GAMEKEEPER_MYSQLRPCTASKSOURCE_H
#include<memory>
#include"XCode/XCode.h"
#include"Protocol/db.pb.h"
#include"Async/RpcTask/RpcTaskSource.h"
using namespace google::protobuf;
namespace Sentry
{
    class RpcHandlerComponent;
    class MysqlRpcTaskSource : public IRpcTask
    {
    public:
        explicit MysqlRpcTaskSource(float timeout = 0)
            : mTimeout(timeout * 1000) { }
        ~MysqlRpcTaskSource() = default;
    public:
        XCode GetCode();
        std::shared_ptr<s2s::Mysql::Response> GetResponse();
    public:
        int GetTimeout() final { return this->mTimeout;}
        long long GetRpcId() final { return mTaskSource.GetTaskId();}
        void OnResponse(std::shared_ptr<com::Rpc_Response> response) final;
    private:
        XCode mCode;
        const int mTimeout;
        RpcHandlerComponent * mRpcComponent;
        TaskSource<std::shared_ptr<s2s::Mysql::Response>> mTaskSource;
    };
}


#endif //GAMEKEEPER_MYSQLRPCTASKSOURCE_H
