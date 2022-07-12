//
// Created by mac on 2022/4/14.
//

#ifndef SERVER_MYSQLCLIENT_H
#define SERVER_MYSQLCLIENT_H

#include"MysqlDefine.h"
#include"Thread/TaskThread.h"
#include"Define/ThreadQueue.h"
#include"Define/ThreadQueue.h"
#include"Coroutine/CoroutineLock.h"
namespace Sentry
{
	class MysqlClient
	{
	 public:
		MysqlClient(MysqlConfig& config);
	 private:
		const MysqlConfig& mConfig;
	};
}
#endif //SERVER_MYSQLCLIENT_H
