//
// Created by mac on 2022/4/14.
//

#include"MysqlClient.h"
#include"MysqlMessage.h"
#include"App/App.h"
#include"Config/MysqlConfig.h"
namespace Sentry
{
	MysqlClient::MysqlClient(IRpc<Mysql::Response> *component)
							 : std::thread(std::bind(&MysqlClient::Update, this)),
                              mComponent(component)
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
		Asio::Context& io = App::Inst()->MainThread();
		this->mLastTime = Helper::Time::NowSecTime();
		const std::string & address = MysqlConfig::Inst()->Address;
		while (!this->mIsClose)
		{
			while (this->GetCommand(command))
			{
				long long rpcId = command->GetRpcId();
				bool res = command->Invoke(this->mMysqlClient, error);
				std::shared_ptr<Mysql::Response> response =
					std::make_shared<Mysql::Response>(res, error, rpcId);
				io.post(std::bind(&IRpc<Mysql::Response>::OnMessage, this->mComponent, address, response));
				this->mLastTime = Helper::Time::NowSecTime();
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
		unsigned short port = MysqlConfig::Inst()->Port;
		const char* ip = MysqlConfig::Inst()->Ip.c_str();
		const char* user = MysqlConfig::Inst()->User.c_str();
		const char* pwd = MysqlConfig::Inst()->Password.c_str();
		const std::string & address = MysqlConfig::Inst()->Address;
		if (!mysql_real_connect(mysql, ip, user, pwd, "", port, NULL, CLIENT_FOUND_ROWS))
		{
			CONSOLE_LOG_ERROR("connect mysql [" << MysqlConfig::Inst()->Address << "] failure");
			return false;
		}
		this->mIsClose = false;
		this->mMysqlClient = mysql;
		Asio::Context& io = App::Inst()->MainThread();
		io.post(std::bind(&IRpc<Mysql::Response>::OnConnectSuccessful, this->mComponent, address));
		return true;
	}
}
