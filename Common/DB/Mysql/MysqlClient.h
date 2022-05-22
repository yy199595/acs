//
// Created by mac on 2022/4/14.
//

#ifndef SERVER_MYSQLCLIENT_H
#define SERVER_MYSQLCLIENT_H

#include"MysqlDefine.h"
#include"MysqlCommandTask.h"
#include"Thread/TaskThread.h"
#include"Define/ThreadQueue.h"
#include"MysqlTableTaskSource.h"
#include"Define/ThreadQueue.h"
#include"Coroutine/CoroutineLock.h"
namespace Sentry
{
	class MysqlClient : public IThread
	{
	 public:
		MysqlClient(MysqlConfig& config);
	 public:
		XCode Start(std::shared_ptr<MysqlAsyncTask> task);
	 public:
		int Start() final;
		void Update() final;
	 private:
		std::thread* mThread;
		MysqlSocket* mMysqlSocket;
		const MysqlConfig& mConfig;
		MultiThread::ConcurrentQueue<std::shared_ptr<MysqlAsyncTask>> mTaskQueue;
	};
}
#endif //SERVER_MYSQLCLIENT_H
