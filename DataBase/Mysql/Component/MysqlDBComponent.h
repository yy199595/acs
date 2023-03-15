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
        MysqlTask(int taskId);
    public:
        void OnResponse(std::shared_ptr<Mysql::Response> response) final;
        std::shared_ptr<Mysql::Response> Await() { return mTask.Await(); }
    private:
        TaskSource<std::shared_ptr<Mysql::Response>> mTask;
    };
}

namespace Sentry
{
    class MysqlClient;
	class MysqlDBComponent : public RpcTaskComponent<int, Mysql::Response>, public IRpc<Mysql::Response>
    {
    public:
        MysqlDBComponent() = default;
        ~MysqlDBComponent() = default;
    public:
        void CloseClients();
        bool Ping(int index = 0);
        bool StartConnectMysql(int count);
		std::shared_ptr<Mysql::Response> Run(std::shared_ptr<Mysql::ICommand> command);
		std::shared_ptr<Mysql::Response> Run(int index, std::shared_ptr<Mysql::ICommand> command);
	 private:
		bool Send(std::shared_ptr<Mysql::ICommand> command);
		bool Send(int index, std::shared_ptr<Mysql::ICommand> command);
		void OnConnectSuccessful(const std::string &address) final;
		void OnMessage(std::shared_ptr<Mysql::Response> message) final;
		void OnTaskComplete(int key) final { this->mNumerPool.Push(key);}
	private:
		std::queue<MysqlClient*> mClients;
		Util::NumberBuilder<int, 1> mNumerPool;
		std::vector<std::unique_ptr<MysqlClient>> mMysqlClients;
    };
}


#endif //SERVER_MYSQLRPCCOMPONENT_H
