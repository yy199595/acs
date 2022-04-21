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
		std::shared_ptr<MysqlAsyncTask> taskSource;
		while (this->mTaskQueue.try_dequeue(taskSource))
		{
			taskSource->Run(this->mMysqlSocket);
		}
		this->HangUp();
	}

	XCode MysqlClient::InitTable(const std::string& pb)
	{
		std::shared_ptr<MysqlTableTaskSource> tableTaskSource
			= std::make_shared<MysqlTableTaskSource>(pb);
		if (!this->mTaskQueue.enqueue(tableTaskSource))
		{
			return XCode::MysqlInitTaskFail;
		}
		this->mThreadVariable.notify_one();
		return tableTaskSource->Await();
	}

	XCode MysqlClient::Invoke(const std::string& sql, s2s::Mysql::Response & response)
	{
		std::shared_ptr<MysqlTaskSource> taskSource
			= std::make_shared<MysqlTaskSource>(sql, response);
		if (!this->mTaskQueue.enqueue(taskSource))
		{
			return XCode::MysqlInitTaskFail;
		}
		this->mThreadVariable.notify_one();
		XCode code = taskSource->Await();
		if (code != XCode::Successful)
		{
			LOG_ERROR(sql);
			LOG_ERROR(response.error());
		}
		else
		{
			LOG_DEBUG(sql);
		}
		return XCode::Successful;
	}

}
