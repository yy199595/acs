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
							 : mComponent(component)
    {
        this->mIndex = 0;
        this->mLastTime = 0;
        this->mIsClose = true;
        this->mTaskCount = 0;
		this->mThread = nullptr;
        this->mMysqlClient = nullptr;
    }

	void MysqlClient::Start()
	{
		this->mThread = new std::thread([this] { Update(); });
		this->mThread->detach();
	}

	void MysqlClient::Update()
	{
		while (!this->StartConnect())
		{
			this->mIndex = 0;
			std::this_thread::sleep_for(std::chrono::seconds(3));
#ifdef __DEBUG__
			CONSOLE_LOG_ERROR("reconnect mysql server");
#endif
		}
		std::string error;
		this->mIsClose = false;
		std::shared_ptr<Mysql::ICommand> command;
		Asio::Context& io = App::Inst()->MainThread();
		this->mLastTime = Helper::Time::NowSecTime();
		while (!this->mIsClose)
		{
			this->WaitPop(command);
			{
				int rpcId = command->GetRpcId();
				this->mLastTime = Helper::Time::NowSecTime();
				std::shared_ptr<Mysql::Response> response =
					std::make_shared<Mysql::Response>(rpcId);
				if (!command->Invoke(this->mMysqlClient, response))
				{

				}
				io.post(std::bind(&IRpc<Mysql::Response>::OnMessage, this->mComponent, response));
			}
		}
		mysql_close(this->mMysqlClient);
	}

	void MysqlClient::Stop()
	{
        this->mIsClose = true;
    }

	bool MysqlClient::StartConnect()
	{
        const MysqlConfig * config = MysqlConfig::Inst();
        if(this->mIndex >= config->Address.size())
        {
            CONSOLE_LOG_ERROR("connect mysql failure");
            return false;
        }
		MYSQL * mysql = mysql_init(NULL);
		const char* user = config->User.c_str();
		const char* pwd = config->Password.c_str();
        const std::string & ip =config->Address[this->mIndex].Ip;
        unsigned short port = config->Address[this->mIndex].Port;
		const std::string & address = config->Address[this->mIndex].FullAddress;
#ifdef __DEBUG__
		CONSOLE_LOG_DEBUG("start connect mysql server [" << address << "]");
#endif // __DEBUG__

		if (!mysql_real_connect(mysql, ip.c_str(), user, pwd, "", port, NULL, CLIENT_FOUND_ROWS))
		{
            this->mIndex++;
#ifdef __DEBUG__
			CONSOLE_LOG_ERROR(mysql_error(mysql));
			//CONSOLE_LOG_ERROR("connect mysql server [" << address << "] failure");
#endif
			return this->StartConnect();
		}
        this->mIndex = 0;
		this->mIsClose = false;
		this->mMysqlClient = mysql;
#ifdef __DEBUG__
		CONSOLE_LOG_DEBUG("connect mysql server [" << address << "]successful");
#endif
		Asio::Context& io = App::Inst()->MainThread();
		io.post(std::bind(&IRpc<Mysql::Response>::OnConnectSuccessful, this->mComponent, address));
		return true;
	}
}
