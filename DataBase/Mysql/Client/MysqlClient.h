//
// Created by mac on 2022/4/14.
//
#ifdef __ENABLE_MYSQL__
#ifndef SERVER_MYSQLCLIENT_H
#define SERVER_MYSQLCLIENT_H
#include<queue>
#include"MysqlDefine.h"
#include"MysqlMessage.h"
#include"Tcp/TcpContext.h"
#include"Config/MysqlConfig.h"
#include"Component/IComponent.h"

namespace Sentry
{
    class MysqlDBComponent;

    class MysqlClient : protected std::thread
    {
    public:
        MysqlClient(IRpc<Mysql::Response> *component);
    public:
        void Stop();
        bool StartConnect();
        void Start() { this->detach(); }
        size_t GetTaskCount() const { return this->mTaskCount; }
        void SendCommand(std::shared_ptr<Mysql::ICommand> command);

    private:
        void Update();
        bool GetCommand(std::shared_ptr<Mysql::ICommand> &command);
    private:
        bool mIsClose;
        std::mutex mLock;
        size_t mTaskCount;
        MYSQL *mMysqlClient;
        long long mLastTime;
		IRpc<Mysql::Response> *mComponent;
        std::queue<std::shared_ptr<Mysql::ICommand>> mCommands;
    };
}
#endif //SERVER_MYSQLCLIENT_H
#endif
