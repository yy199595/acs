//
// Created by zmhy0073 on 2022/7/16.
//

#include "MysqlDBComponent.h"
#include "Entity/Actor/App.h"
#include "Log/Component/LoggerComponent.h"
#include "Server/Config/ServerConfig.h"
#include "Server/Component/ThreadComponent.h"
#include "Timer/Timer/ElapsedTimer.h"
#include "Message/s2s/registry.pb.h"
#include "Lua/Lib/Lib.h"
#include "XCode/XCode.h"
#include "Server/Config/CodeConfig.h"

namespace acs
{
	MysqlDBComponent::MysqlDBComponent()
	{
		this->mLogger = nullptr;

		REGISTER_JSON_CLASS_FIELD(db::Explain, open);
		REGISTER_JSON_CLASS_FIELD(db::Explain, command);

		REGISTER_JSON_CLASS_MUST_FIELD(mysql::Binlog, id);
		REGISTER_JSON_CLASS_MUST_FIELD(mysql::Binlog, name);
		REGISTER_JSON_CLASS_MUST_FIELD(mysql::Binlog, user);
		REGISTER_JSON_CLASS_MUST_FIELD(mysql::Binlog, password);

		REGISTER_JSON_CLASS_FIELD(mysql::Config, db);
		REGISTER_JSON_CLASS_FIELD(mysql::Config, user);
		REGISTER_JSON_CLASS_FIELD(mysql::Config, ping);
		REGISTER_JSON_CLASS_FIELD(mysql::Config, count);
		REGISTER_JSON_CLASS_FIELD(mysql::Config, script);
		REGISTER_JSON_CLASS_FIELD(mysql::Config, debug);
		REGISTER_JSON_CLASS_FIELD(mysql::Config, password);
		REGISTER_JSON_CLASS_MUST_FIELD(mysql::Config, address);

		REGISTER_JSON_CLASS_FIELD(mysql::Explain, id);
		REGISTER_JSON_CLASS_FIELD(mysql::Explain, ref);
		REGISTER_JSON_CLASS_FIELD(mysql::Explain, table);
		REGISTER_JSON_CLASS_FIELD(mysql::Explain, key);
		REGISTER_JSON_CLASS_FIELD(mysql::Explain, select_type);
		REGISTER_JSON_CLASS_FIELD(mysql::Explain, Extra);
		REGISTER_JSON_CLASS_FIELD(mysql::Explain, type);
		REGISTER_JSON_CLASS_FIELD(mysql::Explain, rows);
		REGISTER_JSON_CLASS_FIELD(mysql::Explain, filtered);
		REGISTER_JSON_CLASS_FIELD(mysql::Explain, key_len);
		REGISTER_JSON_CLASS_FIELD(mysql::Explain, possible_keys);

	}

	bool MysqlDBComponent::Awake()
	{
		this->mConfig.count = 1;
		this->mConfig.debug = false;
		LuaCCModuleRegister::Add([](Lua::CCModule & ccModule) {
			ccModule.Open("db.mysql", lua::lib::luaopen_lmysqldb);
		});
		return ServerConfig::Inst()->Get("mysql", this->mConfig);
	}

	bool MysqlDBComponent::LateAwake()
	{
		for(int index = 0; index < this->mConfig.count; index++)
		{
			int id = index + 1;
			timer::ElapsedTimer timer1;
			Asio::Context & main = this->mApp->GetContext();
			tcp::Socket * tcpSocket = this->GetComponent<ThreadComponent>()->CreateSocket(this->mConfig.address);
			std::shared_ptr<mysql::Client> mysqlClient = std::make_shared<mysql::Client>(id, tcpSocket, this, this->mConfig, main);
			{
				int code = mysqlClient->Start();
				if(code != XCode::Ok)
				{
					std::string desc = CodeConfig::Inst()->GetDesc(code);
					LOG_ERROR("mysql [{}] {} ", this->mConfig.address, desc);
					return false;
				}
				this->mFreeClients.Push(id);
				this->mClients.emplace(id, mysqlClient);
				LOG_INFO("[{}ms] connect mysql [{}] ok ", timer1.GetMs(), this->mConfig.address);
			}
		}
		this->mLogger = this->GetComponent<LoggerComponent>();
		return true;
	}

	void MysqlDBComponent::OnDestroy()
	{

	}

	void MysqlDBComponent::OnStart()
	{
		// if(!this->mConfig.binlog.user.empty() && !this->mConfig.binlog.password.empty())
		// {
		// 	json::r::Document document;
		//
		// 	std::unique_ptr<mysql::Response> response = this->Run(std::make_unique<mysql::Request>("SHOW MASTER STATUS"));
		// 	if(response->IsOk() && document.Decode(response->GetFirstResult()))
		// 	{
		// 		std::string binLogFile;
		// 		uint32_t binLogPosition = 4;
		// 		LOG_CHECK_RET(document.Get("File", binLogFile))
		// 		LOG_CHECK_RET(document.Get("Position", binLogPosition))
		//
		// 		mysql::Config config;
		// 		config.user = this->mConfig.binlog.user;
		// 		config.passwd = this->mConfig.binlog.password;
		// 		Asio::Context & main = this->mApp->GetContext();
		// 		ThreadComponent * threadCom = this->GetComponent<ThreadComponent>();
		// 		tcp::Socket * tcpSocket = threadCom->CreateSocket(this->mConfig.address);
		// 		this->mBinLogClient = std::make_shared<mysql::Client>(0, tcpSocket, this, config, main);
		// 		if(this->mBinLogClient->Start() != XCode::Ok)
		// 		{
		// 			LOG_ERROR("binlog mysql client [{}] {} ", this->mConfig.address);
		// 			return;
		// 		}
		// 		if(!this->mBinLogClient->SubBinLog(binLogFile, binLogPosition))
		// 		{
		// 			LOG_ERROR("sub binlog {} fail", binLogFile);
		// 			return;
		// 		}
		// 		LOG_INFO("sub binlog {} ok", binLogFile)
		// 		this->mBinLogClient->StartReceive();
		// 	}
		// }
	}

	void MysqlDBComponent::OnMessage(int id, mysql::Request* request, mysql::Response* response) noexcept
	{
		if(id == 0) //订阅消息
		{
			this->OnBinLog(response);
			this->mBinLogClient->StartReceive();
			return;
		}

		if(this->mMessages.empty())
		{
			this->mFreeClients.Push(id);
		}
		else
		{
			std::unique_ptr<mysql::Request>& req = this->mMessages.front();
			{
				this->Send(id, std::move(req));
			}
			this->mMessages.pop();
		}
		if(!response->IsOk())
		{
			LOG_ERROR("{}", request->ToString());
			LOG_ERROR("{}", response->GetBuffer());
		}
		else
		{
			if (this->mConfig.explain.open)
			{
				std::string cmd;
				if (request->GetCommand(cmd) && cmd != "EXPLAIN" && this->mConfig.explain.HasCommand(cmd))
				{
					std::string sql = request->ToString();
					long long ms = request->GetCostTime();
					this->mApp->StartCoroutine([sql, this, ms]
					{
						this->OnExplain(sql, ms);
					});
				}
			}
			if(this->mConfig.debug)
			{
				CONSOLE_LOG_DEBUG("request=>{}", request->ToString());
				CONSOLE_LOG_DEBUG("[{}ms] response=>{}", request->GetCostTime(), response->GetBuffer());
			}
		}
		int rpcId = request->GetRpcId();
		this->OnResponse(rpcId, std::unique_ptr<mysql::Response>(response));
	}

	void MysqlDBComponent::OnExplain(const std::string& sql, long long ms) noexcept
	{
		const std::string newSql = fmt::format("EXPLAIN {}", sql);
		std::unique_ptr<mysql::Response> response1 = this->Run(newSql);
		{
			std::string result;
			if(response1->GetFirstResult(result))
			{
				std::unique_ptr<custom::LogInfo> sqlLog = std::make_unique<custom::LogInfo>();
				{
					sqlLog->Level = custom::LogLevel::None;
					sqlLog->Content = fmt::format("[{}ms] {}\n", ms, sql);
					{
						sqlLog->Content.append(result);
						this->mLogger->PushLog("mysql", std::move(sqlLog));
					}
				}
			}
		}
	}


	void MysqlDBComponent::OnBinLog(mysql::Response* response) noexcept
	{
		LOG_DEBUG("{}", response->GetErrorCode());
	}


	void MysqlDBComponent::Send(std::unique_ptr<mysql::Request> request, int& rpcId)
	{
		rpcId = this->BuildRpcId();
		{
			request->SetRpcId(rpcId);
			this->Send(std::move(request));
		}
	}

	void MysqlDBComponent::Send(int id, std::unique_ptr<mysql::Request> request)
	{
		auto iter = this->mClients.find(id);
		if(iter == this->mClients.end())
		{
			return;
		}
		iter->second->Send(std::move(request));
	}

	void MysqlDBComponent::Send(std::unique_ptr<mysql::Request> request)
	{
		int id = 0;
		if(!this->mFreeClients.Pop(id))
		{
			this->mMessages.emplace(std::move(request));
			return;
		}
		this->Send(id, std::move(request));
	}

	std::unique_ptr<mysql::Response> MysqlDBComponent::Run(const std::string& sql)
	{
		return this->Run(std::make_unique<mysql::Request>(sql));
	}


	std::unique_ptr<mysql::Response> MysqlDBComponent::Run(std::unique_ptr<mysql::Request> request)
	{
		int rpcId = this->BuildRpcId();
		{
			request->SetRpcId(rpcId);
			this->Send(std::move(request));
			return this->BuildRpcTask<MysqlTask>(rpcId)->Await();
		}
	}
}