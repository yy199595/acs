//
// Created by mac on 2022/4/14.
//

#include"MysqlClient.h"
#include"Component/Mysql/MysqlRpcComponent.h"
#include"MysqlMessage.h"
#include"Util/sha1.h"
namespace Sentry
{
	MysqlClient::MysqlClient(const MysqlConfig &config, MysqlRpcComponent *component)
							 : mConfig(config), mComponent(component)
	{		
		this->mIsClose = true;
		this->mThread = nullptr;
		this->mMysqlClient = nullptr;
	}

	void MysqlClient::Update()
	{
		std::string error;
		std::shared_ptr<Mysql::ICommand> command;
		while (!this->mIsClose)
		{
			while (this->GetCommand(command))
			{				
				MYSQL_RES * result = command->Invoke(this->mMysqlClient, error);			
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}

	void MysqlClient::Stop()
	{
		std::lock_guard<std::mutex> lock(this->mLock);
	}

	void MysqlClient::SendCommand(std::shared_ptr<Mysql::ICommand> command)
	{
		std::lock_guard<std::mutex> lock(this->mLock);
		this->mCommands.emplace(std::move(command));
	}

	bool MysqlClient::GetCommand(std::shared_ptr<Mysql::ICommand>& command)
	{
		std::lock_guard<std::mutex> lock(this->mLock);
		if (this->mCommands.empty())
		{
			return false;
		}
		command = this->mCommands.front();
		this->mCommands.pop();
		return true;
	}

	bool MysqlClient::StartConnect()
	{
		std::lock_guard<std::mutex> lock(this->mLock);
		MYSQL * mysql = mysql_init((MYSQL*)0);	
		unsigned short port = this->mConfig.mPort;
		const char* ip = this->mConfig.mIp.c_str();
		const char* user = this->mConfig.mUser.c_str();
		const char* pwd = this->mConfig.mPassword.c_str();
		if (!mysql_real_connect(mysql, ip, user, pwd, "", port, NULL, CLIENT_FOUND_ROWS))
		{
			CONSOLE_LOG_ERROR("connect mysql [" << this->mConfig.mAddress << "] failure");
			return false;
		}
		this->mIsClose = false;
		this->mMysqlClient = mysql;
		this->mThread = new std::thread(std::bind(&MysqlClient::Update, this));
		CONSOLE_LOG_INFO("connect mysql [" << this->mConfig.mAddress << "] successful");
		return true;
	}
}
