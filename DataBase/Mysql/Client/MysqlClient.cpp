//
// Created by mac on 2022/4/14.
//

#include"MysqlClient.h"
#include"MysqlMessage.h"
#include"Component/MysqlDBComponent.h"
namespace Sentry
{
	MysqlClient::MysqlClient(const MysqlConfig &config, MysqlDBComponent *component)
							 : mConfig(config), mComponent(component),
                              std::thread(std::bind(&MysqlClient::Update, this))
    {
        this->mLastTime = 0;
        this->mIsClose = true;
        this->mTaskCount = 0;
        this->mMysqlClient = nullptr;
    }

	void MysqlClient::Update()
    {
        std::string error;
        this->mIsClose = !this->StartConnect();
        std::shared_ptr<Mysql::ICommand> command;
        Asio::Context &io = App::Get()->GetThread();
        this->mLastTime = Helper::Time::GetNowSecTime();
        while (!this->mIsClose)
        {
            while (this->GetCommand(command))
            {
                std::shared_ptr<Mysql::Response> response;
                bool res = command->Invoke(this->mMysqlClient, error);
                response = std::make_shared<Mysql::Response>(res, error);
                if (command->GetRpcId() != 0)
                {
                    long long rpcId = command->GetRpcId();
                    io.post(std::bind(&MysqlDBComponent::OnResponse, this->mComponent, rpcId, response));
                }
                this->mLastTime = Helper::Time::GetNowSecTime();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        mysql_close(this->mMysqlClient);
    }

	void MysqlClient::Stop()
	{
        std::lock_guard<std::mutex> lock(this->mLock);
        this->mIsClose = true;
    }

	void MysqlClient::SendCommand(std::shared_ptr<Mysql::ICommand> command)
	{
		std::lock_guard<std::mutex> lock(this->mLock);

        this->mTaskCount++;
		this->mCommands.emplace(std::move(command));
	}

	bool MysqlClient::GetCommand(std::shared_ptr<Mysql::ICommand>& command)
	{
		std::lock_guard<std::mutex> lock(this->mLock);
		if (this->mCommands.empty())
		{
			return false;
		}
        this->mTaskCount--;
        command = this->mCommands.front();
		this->mCommands.pop();
		return true;
	}

	bool MysqlClient::StartConnect()
	{
		MYSQL * mysql = mysql_init(NULL);
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
		CONSOLE_LOG_INFO("connect mysql [" << this->mConfig.mAddress << "] successful");
		return true;
	}
}
