//
// Created by mac on 2022/4/14.
//
#ifdef __ENABLE_MYSQL__


#include"MysqlClient.h"
#include"MysqlMessage.h"
#include"Entity/Actor/App.h"
#include"Mysql/Config/MysqlConfig.h"
namespace Tendo
{
	MysqlClient::MysqlClient(IRpc<Mysql::Response> *component, const MysqlConfig & config)
							 : mComponent(component), mConfig(config)
    {
		this->mLastTime = 0;
		this->mThread = nullptr;
        this->mMysqlClient = nullptr;
    }

	void MysqlClient::Start()
	{
		if (this->mThread == nullptr)
		{
			this->mThread = new std::thread([this] { Update(); });
			this->mThread->detach();
		}
	}

	void MysqlClient::Update()
	{
		while (!this->StartConnect())
		{
			std::this_thread::sleep_for(std::chrono::seconds(3));
#ifdef __DEBUG__
			CONSOLE_LOG_ERROR("reconnect mysql server");
#endif
		}
		std::string error;
		std::shared_ptr<Mysql::ICommand> command;
		this->mLastTime = Helper::Time::NowSecTime();
		Asio::Context& io = App::Inst()->MainThread();
		std::shared_ptr<MysqlClient> self = this->shared_from_this();
		while (true)
		{
			this->WaitPop(command);
			this->mLastTime = Helper::Time::NowSecTime();
			if (command->GetRpcId() == -1) //ֹͣ����
			{
				break;
			}
			else if(command->GetRpcId() > 0)			
			{
				int rpcId = command->GetRpcId();
				std::shared_ptr<Mysql::Response> response =
					std::make_shared<Mysql::Response>(rpcId);
				if (!command->Invoke(this->mMysqlClient, response))
				{
					std::string sql;
					command->GetSql(sql);
					CONSOLE_LOG_ERROR("sql : " << sql);
					CONSOLE_LOG_ERROR("err : " << response->GetError());
				}
				io.post(std::bind(&IRpc<Mysql::Response>::OnMessage, this->mComponent, response));
			}
			else
			{
				std::shared_ptr<Mysql::Response> response =
					std::make_shared<Mysql::Response>(0);
				command->Invoke(this->mMysqlClient, response);		
			}
		}
		mysql_close(this->mMysqlClient);
		CONSOLE_LOG_INFO("close mysql client successful");
	}

	void MysqlClient::Stop()
	{
		std::shared_ptr<Mysql::ICommand>
		    stopCommand = std::make_shared<Mysql::StopCommand>();
		stopCommand->SetRpcId(-1);
		this->Push(stopCommand);
    }

	void MysqlClient::Ping()
	{
		std::shared_ptr<Mysql::ICommand>
			pingCommand = std::make_shared<Mysql::PingCommand>();
		pingCommand->SetRpcId(0);
		this->Push(pingCommand);
	}

	bool MysqlClient::StartConnect()
	{
		MYSQL* mysql = mysql_init(nullptr);
		const char* user = this->mConfig.User.c_str();
		const char* pwd = this->mConfig.Password.c_str();
		const char * ip = this->mConfig.Address.Ip.c_str();
		unsigned short port = this->mConfig.Address.Port;
		const std::string& address = this->mConfig.Address.FullAddress;
#ifdef __DEBUG__
		CONSOLE_LOG_DEBUG("start connect mysql server [" << address << "]");
#endif // __DEBUG__

		if (!mysql_real_connect(mysql, ip, user, pwd, "", port, NULL, CLIENT_FOUND_ROWS))
		{
#ifdef __DEBUG__
			CONSOLE_LOG_ERROR(mysql_error(mysql));
			//CONSOLE_LOG_ERROR("connect mysql server [" << address << "] failure");
#endif
			return this->StartConnect();
		}
		this->mMysqlClient = mysql;
#ifdef __DEBUG__
		CONSOLE_LOG_DEBUG("connect mysql server [" << address << "]successful");
#endif
		Asio::Context& io = App::Inst()->MainThread();
		io.post(std::bind(&IRpc<Mysql::Response>::OnConnectSuccessful, this->mComponent, address));
		return true;
	}
	std::shared_ptr<Mysql::Response> MysqlClient::Run(const std::shared_ptr<Mysql::ICommand>& command)
	{
		if (this->mThread != nullptr)
		{
			return nullptr;
		}
		std::shared_ptr<Mysql::Response> response =
			std::make_shared<Mysql::Response>(0);
		if (!command->Invoke(this->mMysqlClient, response))
		{
			std::string sql;
			command->GetSql(sql);
			CONSOLE_LOG_ERROR("sql : " << sql);
			CONSOLE_LOG_ERROR("err : " << response->GetError());
		}
		return response;
	}
	MysqlClient::~MysqlClient()
	{
		if(this->mThread != nullptr)
		{
			delete this->mThread;
			this->mThread = nullptr;
		}
	}
}

#endif
