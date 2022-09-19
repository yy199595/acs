//
// Created by zmhy0073 on 2022/7/16.
//

#ifndef SERVER_MYSQLRPCCOMPONENT_H
#define SERVER_MYSQLRPCCOMPONENT_H

#include"../Client/MysqlDefine.h"
#include"../Client/MysqlMessage.h"
#include"Util/NumberBuilder.h"
#include"Component/Rpc/RpcTaskComponent.h"

namespace Sentry
{
    class MysqlTask : public IRpcTask<Mysql::Response>
    {
    public:
        MysqlTask(int taskId, int ms);
    public:
        long long GetRpcId() final { return this->mTaskId; }

    public:
        void OnTimeout() final;
        void OnResponse(std::shared_ptr<Mysql::Response> response) final;
        std::shared_ptr<Mysql::Response> Await() { return mTask.Await(); }
    private:
        int mTaskId;
        TaskSource<std::shared_ptr<Mysql::Response>> mTask;
    };
}

namespace Sentry
{
    class MysqlClient;
    class MysqlDBComponent : public RpcTaskComponent<Mysql::Response>, public IStart
    {
    public:
        MysqlDBComponent() = default;
        ~MysqlDBComponent() = default;
    public:
        bool Ping(int index = 0);
        std::shared_ptr<MysqlClient> GetClient(int index = -1);
        std::shared_ptr<Mysql::Response> Run(std::shared_ptr<MysqlClient> client, std::shared_ptr<Mysql::ICommand> command);
    private:
        bool OnStart() final;
        bool LateAwake() final;
        void OnDelTask(long long taskId, RpcTask task) final;
    private:
		MysqlConfig mConfig;
        Util::NumberBuilder<int, 10> mNumberPool;
		std::vector<std::shared_ptr<MysqlClient>> mMysqlClients;
    };
}


#endif //SERVER_MYSQLRPCCOMPONENT_H
