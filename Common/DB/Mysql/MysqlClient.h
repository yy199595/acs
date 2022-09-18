//
// Created by mac on 2022/4/14.
//

#ifndef SERVER_MYSQLCLIENT_H
#define SERVER_MYSQLCLIENT_H
#include"mysql.h"
#include"MysqlDefine.h"
#include"Network/TcpContext.h"
namespace Sentry
{
	class MysqlRpcComponent;
	class MysqlClient : public std::thread
	{
	 public:
		MysqlClient(MysqlConfig& config, MysqlRpcComponent* component);
	 public:
		bool StartConnect();
	 private:
		void Update();
	 private:
		std::mutex mLock;
		MYSQL * mMysqlClient;
		const MysqlConfig& mConfig;
		MysqlRpcComponent* mComponent;
	};
}
#endif //SERVER_MYSQLCLIENT_H
