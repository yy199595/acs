//
// Created by mac on 2022/4/14.
//

#ifndef SERVER_MYSQLCLIENT_H
#define SERVER_MYSQLCLIENT_H

#include"MysqlDefine.h"
#include"MysqlMessage.h"
#include"Network/TcpContext.h"
namespace Sentry
{
	class MysqlRpcComponent;
	class MysqlClient
	{
	 public:
		MysqlClient(const MysqlConfig& config, MysqlRpcComponent* component);
	 public:
		 void Stop();
		bool StartConnect();
		void SendCommand(std::shared_ptr<Mysql::ICommand> command);
	 private:
		void Update();
		bool GetCommand(std::shared_ptr<Mysql::ICommand> & command);
	 private:
		 bool mIsClose;
		std::mutex mLock;
		MYSQL * mMysqlClient;
		std::thread* mThread;
		const MysqlConfig& mConfig;
		MysqlRpcComponent* mComponent;
		std::queue<std::shared_ptr<Mysql::ICommand>> mCommands;
	};
}
#endif //SERVER_MYSQLCLIENT_H
