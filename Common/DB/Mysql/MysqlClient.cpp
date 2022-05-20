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
		this->mThread = nullptr;
		this->mLock = std::make_shared<CoroutineLock>();
	}

	int MysqlClient::Start()
	{
		std::shared_ptr<TaskSource<int>> taskSource(new TaskSource<int>);
		this->mThread = new std::thread([this, taskSource]()
		{
			this->mThreadId = std::this_thread::get_id();
			unsigned short port = this->mConfig.mPort;
			const char * ip = this->mConfig.mIp.c_str();
			const char * user = this->mConfig.mUser.c_str();
			const char * password = this->mConfig.mPassword.c_str();
			MysqlSocket* mysqlSocket1 = mysql_init((MYSQL*)nullptr);
			this->mMysqlSocket = mysql_real_connect(mysqlSocket1, ip, user, password,
					nullptr, port, nullptr,CLIENT_MULTI_STATEMENTS);

			int value = 1;
			mysql_options(mysqlSocket1, MYSQL_OPT_RECONNECT, &value); // 自动重连
			if (this->mMysqlSocket == nullptr)
			{
				taskSource->SetResult(-1);
				return;
			}
			taskSource->SetResult(0);
			while(!this->mIsClose)
			{
				this->Update();
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		});
		this->mThread->detach();
		return taskSource->Await();
	}

	void MysqlClient::Update()
	{
		if(this->mCurTask != nullptr)
		{
			this->mCurTask->Run(this->mMysqlSocket);
			this->mCurTask = nullptr;
		}
		this->HangUp();
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
