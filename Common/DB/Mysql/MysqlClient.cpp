//
// Created by mac on 2022/4/14.
//

#include"MysqlClient.h"
#include"Async/TaskSource.h"
namespace Sentry
{
	MysqlClient::MysqlClient(MysqlConfig & config)
		: mConfig(config), IThread("Mysql"),
		mThread(new thread(std::bind(&MysqlClient::Update, this)))
	{
		this->mThread->detach();
	}

	int MysqlClient::Start()
	{
		int value = 1;
		unsigned short port = this->mConfig.mPort;
		const char* ip = this->mConfig.mIp.c_str();
		const char* user = this->mConfig.mUser.c_str();
		const char* password = this->mConfig.mPassword.c_str();
		MysqlSocket* mysqlSocket1 = mysql_init((MYSQL*)nullptr);
		this->mMysqlSocket = mysql_real_connect(mysqlSocket1, ip, user, password,
			nullptr, port, nullptr, CLIENT_MULTI_STATEMENTS);
		mysql_options(mysqlSocket1, MYSQL_OPT_RECONNECT, &value); // 自动重连
		return this->mMysqlSocket != nullptr ? 0 : -1;
	}


	void MysqlClient::Update()
	{
		this->mThreadId = std::this_thread::get_id();
		while(!this->mIsClose)
		{
			std::shared_ptr<MysqlAsyncTask> mysqlAsyncTask;
			if(this->mTaskQueue.try_dequeue(mysqlAsyncTask))
			{
				mysqlAsyncTask->Run(this->mMysqlSocket);
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		CONSOLE_LOG_FATAL("close mysql thread");
	}

	XCode MysqlClient::Start(std::shared_ptr<MysqlAsyncTask> task)
	{
		if(!task->Init())
		{
			return XCode::MysqlInitTaskFail;
		}
		this->mTaskQueue.enqueue(task);
		return task->Await();
	}
}
