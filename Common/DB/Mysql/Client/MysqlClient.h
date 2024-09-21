//
// Created by mac on 2022/4/14.
//
#ifdef __ENABLE_MYSQL__
#ifndef SERVER_MYSQLCLIENT_H
#define SERVER_MYSQLCLIENT_H
#include<queue>
#include"MysqlDefine.h"
#include"MysqlMessage.h"
#include"Core/Thread/AsioThread.h"
#include"Network/Tcp/TcpClient.h"
#include"Core/Queue/ThreadQueue.h"
#include"Mysql/Config/MysqlConfig.h"
#include"Entity/Component/IComponent.h"

namespace acs
{
    class MysqlDBComponent;

	class MysqlClient
    {
    public:
		typedef IRpc<Mysql::IRequest, Mysql::Response> Component;
        explicit MysqlClient(Component *component, const mysql::MysqlConfig & config);
    public:
        void Stop();
        void Ping();
		bool Start();
		void Push(std::unique_ptr<Mysql::IRequest> command);
        std::unique_ptr<Mysql::Response> Sync(std::unique_ptr<Mysql::IRequest> command);
    private:
		bool StartConnect();
	private:
        MYSQL *mMysqlClient;
		Component *mComponent;
		custom::AsioThread mThread;
		const mysql::MysqlConfig & mConfig;
    };
}
#endif //SERVER_MYSQLCLIENT_H
#endif
