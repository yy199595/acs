//
// Created by mac on 2022/4/14.
//

#include"MysqlClient.h"
#include"Async/TaskSource.h"
namespace Sentry
{
	MysqlClient::MysqlClient(MysqlConfig & config)
		: mConfig(config), IThread("Mysql")
	{

	}

	int MysqlClient::Start()
	{
		int value = 1;
		printf("start connect mysql ....\n");
		unsigned short port = this->mConfig.mPort;
		const char* ip = this->mConfig.mIp.c_str();
		const char* user = this->mConfig.mUser.c_str();
		const char* password = this->mConfig.mPassword.c_str();
		MysqlSocket* mysqlSocket1 = mysql_init((MYSQL*)nullptr);
		this->mMysqlSocket = mysql_real_connect(mysqlSocket1, ip, user, password,
			nullptr, port, nullptr, CLIENT_MULTI_STATEMENTS);
		mysql_options(mysqlSocket1, MYSQL_OPT_RECONNECT, &value); // 自动重连
		if(this->mMysqlSocket != nullptr)
		{
			this->mThread = new std::thread(std::bind(&MysqlClient::Update, this));
			this->mThread->detach();
			return 0;
		}
		return -1;
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
	}

	XCode MysqlClient::Start(std::shared_ptr<MysqlAsyncTask> task)
	{
		XCode code = task->Init();
		if(code != XCode::Successful)
		{
			return code;
		}
		this->mTaskQueue.enqueue(task);
		return task->Await();
	}
}
