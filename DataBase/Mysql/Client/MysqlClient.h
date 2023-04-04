//
// Created by mac on 2022/4/14.
//
#ifdef __ENABLE_MYSQL__
#ifndef SERVER_MYSQLCLIENT_H
#define SERVER_MYSQLCLIENT_H
#include<queue>
#include"MysqlDefine.h"
#include"MysqlMessage.h"
#include"Network/Tcp/TcpContext.h"
#include"Core/Queue/ThreadQueue.h"
#include"Mysql/Config/MysqlConfig.h"
#include"Core/Component/IComponent.h"

namespace Tendo
{
    class MysqlDBComponent;

	class MysqlClient : public ThreadQueue<std::shared_ptr<Mysql::ICommand>>,
			public std::enable_shared_from_this<MysqlClient>
    {
    public:
        explicit MysqlClient(IRpc<Mysql::Response> *component, const MysqlConfig & config);
    public:
        void Stop();
		void Start();
		bool StartConnect();
    private:
        void Update();
    private:
        size_t mIndex;
        MYSQL *mMysqlClient;
		std::thread * mThread;
		const MysqlConfig & mConfig;
		IRpc<Mysql::Response> *mComponent;
    };
}
#endif //SERVER_MYSQLCLIENT_H
#endif
