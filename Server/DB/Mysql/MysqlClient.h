//
// Created by mac on 2022/4/14.
//

#ifndef SERVER_MYSQLCLIENT_H
#define SERVER_MYSQLCLIENT_H
#include"MysqlDefine.h"
#include"MysqlTaskSource.h"
#include"Thread/TaskThread.h"
#include"Define/ThreadQueue.h"
#include"MysqlTableTaskSource.h"
namespace Sentry
{
	class MysqlClient : public IThread
	{
	public:
		MysqlClient(MysqlConfig & config);
	public:
		XCode InitTable(const std::string & pb);
		XCode Invoke(const std::string & sql, s2s::Mysql::Response & response);
	 public:
		int Start() final;
		void Update() final;

	private:
		std::thread * mThread;
		std::atomic_bool mIsClose;
		const MysqlConfig & mConfig;
		MysqlSocket * mMysqlSocket;
		MultiThread::ConcurrentQueue<std::shared_ptr<MysqlAsyncTask>> mTaskQueue;
	};
}


#endif //SERVER_MYSQLCLIENT_H
