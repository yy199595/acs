//
// Created by zmhy0073 on 2022/7/16.
//

#ifndef SERVER_MYSQLRPCCOMPONENT_H
#define SERVER_MYSQLRPCCOMPONENT_H

#include"Client/MysqlDefine.h"
#include"Client/MysqlMessage.h"
#include"Guid/NumberBuilder.h"
#include"Component/RpcTaskComponent.h"

namespace Sentry
{
    class MysqlTask : public IRpcTask<Mysql::Response>
    {
    public:
        MysqlTask(long long taskId, int ms);
    public:
        long long GetRpcId() final { return this->mTaskId; }

    public:
        void OnTimeout() final;
        void OnResponse(std::shared_ptr<Mysql::Response> response) final;
        std::shared_ptr<Mysql::Response> Await() { return mTask.Await(); }
    private:
        long long mTaskId;
        TaskSource<std::shared_ptr<Mysql::Response>> mTask;
    };
}

namespace Sentry
{
    class MysqlClient;
    class MysqlDBComponent : public RpcTaskComponent<Mysql::Response>
    {
    public:
        MysqlDBComponent() = default;
        ~MysqlDBComponent() = default;
    public:
        void CloseClients();
        bool Ping(int index = 0);
        bool StartConnectMysql();
        std::shared_ptr<MysqlClient> GetClient(int index = -1);
        bool Run(std::shared_ptr<MysqlClient> client, std::shared_ptr<Mysql::ICommand> command);
    private:
        bool LoadConfig();
    private:
		MysqlConfig mConfig;
		std::vector<std::shared_ptr<MysqlClient>> mMysqlClients;
    };
}


#endif //SERVER_MYSQLRPCCOMPONENT_H
