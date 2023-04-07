//
// Created by mac on 2022/4/14.
//
#ifdef __ENABLE_MYSQL__


#include"MysqlClient.h"
#include"MysqlMessage.h"
#include"Entity/App/App.h"
#include"Mysql/Config/MysqlConfig.h"
namespace Tendo
{
	MysqlClient::MysqlClient(IRpc<Mysql::Response> *component, const MysqlConfig & config)
							 : mComponent(component), mConfig(config)
    {
        this->mIndex = 0;
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
			this->mIndex = 0;
			std::this_thread::sleep_for(std::chrono::seconds(3));
#ifdef __DEBUG__
			CONSOLE_LOG_ERROR("reconnect mysql server");
#endif
		}
		std::string error;
		std::shared_ptr<Mysql::ICommand> command;
		Asio::Context& io = App::Inst()->MainThread();
		std::shared_ptr<MysqlClient> self = this->shared_from_this();
		while (true)
		{
			this->WaitPop(command);
			int rpcId = command->GetRpcId();
			if (rpcId > 0)
			{
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
				break;
			}
		}
		mysql_close(this->mMysqlClient);
		CONSOLE_LOG_INFO("close mysql client successful");
	}

	void MysqlClient::Stop()
	{
		std::shared_ptr<Mysql::ICommand>
		    stopCommand = std::make_shared<Mysql::StopCommand>();
		this->Push(stopCommand);
    }

	bool MysqlClient::StartConnect()
	{
		if (this->mIndex >= this->mConfig.Address.size())
		{
			CONSOLE_LOG_ERROR("connect mysql failure");
			return false;
		}
		MYSQL* mysql = mysql_init(nullptr);
		const char* user = this->mConfig.User.c_str();
		const char* pwd = this->mConfig.Password.c_str();
		const std::string& ip = this->mConfig.Address[this->mIndex].Ip;
		unsigned short port = this->mConfig.Address[this->mIndex].Port;
		const std::string& address = this->mConfig.Address[this->mIndex].FullAddress;
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
		this->mMysqlClient = mysql;
#ifdef __DEBUG__
		CONSOLE_LOG_DEBUG("connect mysql server [" << address << "]successful");
#endif
		Asio::Context& io = App::Inst()->MainThread();
		io.post(std::bind(&IRpc<Mysql::Response>::OnConnectSuccessful, this->mComponent, address));
		return true;
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
