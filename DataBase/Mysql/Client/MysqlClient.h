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
#include"Queue/ThreadQueue.h"
#include"Config/MysqlConfig.h"
#include"Component/IComponent.h"

namespace Sentry
{
    class MysqlDBComponent;

	class MysqlClient : public ThreadQueue<std::shared_ptr<Mysql::ICommand>>
    {
    public:
        explicit MysqlClient(IRpc<Mysql::Response> *component);
    public:
        void Stop();
		void Start();
		bool StartConnect();
    private:
        void Update();
    private:
        bool mIsClose;
        size_t mIndex;
        size_t mTaskCount;
        MYSQL *mMysqlClient;
        long long mLastTime;
		std::thread * mThread;
		IRpc<Mysql::Response> *mComponent;
    };
}
#endif //SERVER_MYSQLCLIENT_H
#endif
