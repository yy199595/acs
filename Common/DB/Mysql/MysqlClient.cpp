//
// Created by mac on 2022/4/14.
//

#include"MysqlClient.h"
#include"Async/TaskSource.h"
namespace Sentry
{
	MysqlClient::MysqlClient(MysqlConfig & config)
		: mConfig(config), IThread("Mysql"), mThread(nullptr)
	{
		this->mLock = std::make_shared<CoroutineLock>();
		this->mThread = new std::thread(std::bind(&MysqlClient::Update, this));
		this->mThread->detach();
	}

	int MysqlClient::Start()
	{
		int value = 1;
		unsigned short port = this->mConfig.mPort;
		const char* ip = this->mConfig.mIp.c_str();
		this->mThreadId = std::this_thread::get_id();
		const char* user = this->mConfig.mUser.c_str();
		const char* password = this->mConfig.mPassword.c_str();
		MysqlSocket* mysqlSocket1 = mysql_init((MYSQL*)nullptr);
		this->mMysqlSocket = mysql_real_connect(mysqlSocket1, ip, user, password,
			nullptr, port, nullptr, CLIENT_MULTI_STATEMENTS);
		mysql_options(mysqlSocket1, MYSQL_OPT_RECONNECT, &value); // 自动重连
		if(this->mMysqlSocket != nullptr)
		{
			this->mThreadVariable.notify_one();
			return 0;
		}
		return -1;
	}


	void MysqlClient::Update()
	{
		this->HangUp();
		while(!this->mIsClose && this->mMysqlSocket)
		{
			if(this->mCurTask != nullptr)
			{
				this->mCurTask->Run(this->mMysqlSocket);
				this->mCurTask = nullptr;
			}
			this->HangUp();
		}
	}

	XCode MysqlClient::InitTable(const std::string& pb)
	{
		std::shared_ptr<MysqlTableTaskSource> tableTaskSource
			= std::make_shared<MysqlTableTaskSource>(pb);
		return this->Start(tableTaskSource);
	}

	XCode MysqlClient::Start(std::shared_ptr<MysqlAsyncTask> task)
	{
		XCode code = task->Init();
		if(code != XCode::Successful)
		{
			return code;
		}
		AutoCoroutineLock lock(this->mLock);
		this->mCurTask = task;
		this->mThreadVariable.notify_one();
		return task->Await();
	}
}
