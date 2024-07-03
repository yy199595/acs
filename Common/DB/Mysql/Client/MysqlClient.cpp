//
// Created by mac on 2022/4/14.
//
#ifdef __ENABLE_MYSQL__


#include"MysqlClient.h"
#include"MysqlMessage.h"
#include"Entity/Actor/App.h"
#include"Util/String/String.h"
#include"Core/Thread/ThreadSync.h"
#include"Mysql/Config/MysqlConfig.h"
namespace joke
{
	MysqlClient::MysqlClient(Component* component, const mysql::MysqlConfig& config)
			: mComponent(component), mConfig(config)
	{
		this->mThread.Start(100, "mysql");
		this->mMysqlClient = nullptr;
	}

	bool MysqlClient::Start()
	{
		custom::ThreadSync<bool> threadSync;
		Asio::Context & io = this->mThread.Context();
		io.post([this, &threadSync]
		{
			int count = 0;
			std::chrono::seconds sleep(1);
			do
			{
				count++;
				if(this->StartConnect())
				{
					threadSync.SetResult(true);
					return;
				}
				std::this_thread::sleep_for(sleep);
			}
			while (count < 3);
			threadSync.SetResult(false);
		});
		return threadSync.Wait();
	}

	void MysqlClient::Stop()
	{

    }

	void MysqlClient::Ping()
	{
		std::unique_ptr<Mysql::IRequest>
			pingCommand = std::make_unique<Mysql::PingRequest>();
		pingCommand->SetRpcId(0);
		this->Push(std::move(pingCommand));
	}

	bool MysqlClient::StartConnect()
	{
		std::string ip;
		unsigned short port = 0;
		MYSQL* mysql = mysql_init(nullptr);
		const char* user = this->mConfig.User.c_str();
		const char* pwd = this->mConfig.Password.c_str();
		if(!help::Str::SplitAddr(this->mConfig.Address, ip, port))
		{
			return false;
		}
		if (!mysql_real_connect(mysql, ip.c_str(), user, pwd, "", port, nullptr, CLIENT_FOUND_ROWS))
		{
#ifdef __DEBUG__
			CONSOLE_LOG_ERROR(mysql_error(mysql));
			//CONSOLE_LOG_ERROR("connect mysql server [" << address << "] failure");
#endif
			return false;
		}
		if(!this->mConfig.DB.empty())
		{
			const char * db = this->mConfig.DB.c_str();
			if(mysql_select_db(mysql, db) != 0)
			{
				std::string sql = fmt::format("CREATE DATABASE {}", db);
				if(mysql_real_query(mysql, sql.c_str(), sql.size()) != 0)
				{
					LOG_ERROR("{}", mysql_error(mysql));
					return false;
				}
			}

		}
		this->mMysqlClient = mysql;
		return true;
	}

	void MysqlClient::Push(std::unique_ptr<Mysql::IRequest> command)
	{
		Asio::Context& io = this->mThread.Context();
		io.post([this, request = command.release()]
		{
			Mysql::Response * response = new Mysql::Response();
			if (!request->Invoke(this->mMysqlClient, response))
			{

			}
			if(this->mComponent == nullptr)
			{
				delete request;
				delete response;
				return;
			}
			Asio::Context & t = App::Inst()->GetContext();
			t.post([this, request, response] { this->mComponent->OnMessage(request, response); });
		});
	}

	std::unique_ptr<Mysql::Response> MysqlClient::Sync(std::unique_ptr<Mysql::IRequest> command)
	{
		custom::ThreadSync<bool> threadSync;
		std::unique_ptr<Mysql::Response> response;
		Asio::Context & io = this->mThread.Context();
		asio::post(io, [this, &command, &threadSync, &response]
		{
			response = std::make_unique<Mysql::Response>();
			if(!command->Invoke(this->mMysqlClient, response.release()))
			{
				std::string sql;
				command->GetSql(sql);
				CONSOLE_LOG_ERROR("sql : {}", sql);
				CONSOLE_LOG_ERROR("err : {}", response->GetError());
			}
			threadSync.SetResult(true);
		});
		threadSync.Wait();
		return response;
	}
}

#endif
