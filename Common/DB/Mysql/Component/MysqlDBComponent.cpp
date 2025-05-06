//
// Created by zmhy0073 on 2022/7/16.
//

#include "MysqlDBComponent.h"
#include "Entity/Actor/App.h"
#include "Util/File/FileHelper.h"
#include "Log/Component/LoggerComponent.h"
#include "Server/Config/ServerConfig.h"
#include "Server/Component/ThreadComponent.h"
#include "Timer/Timer/ElapsedTimer.h"
#include "Message/s2s/registry.pb.h"
#include "Lua/Lib/Lib.h"
#include "XCode/XCode.h"
#include "Server/Config/CodeConfig.h"
#include "Util/Tools/Math.h"
namespace acs
{
	MysqlDBComponent::MysqlDBComponent()
	{
		this->mCount = 0;
		this->mRetryCount = 0;
		REGISTER_JSON_CLASS_FIELD(db::Explain, open);
		REGISTER_JSON_CLASS_FIELD(db::Explain, command);

		REGISTER_JSON_CLASS_FIELD(mysql::Cluster, ping);
		REGISTER_JSON_CLASS_FIELD(mysql::Cluster, count);
		REGISTER_JSON_CLASS_FIELD(mysql::Cluster, retry);
		REGISTER_JSON_CLASS_FIELD(mysql::Cluster, script);
		REGISTER_JSON_CLASS_FIELD(mysql::Cluster, debug);
		REGISTER_JSON_CLASS_FIELD(mysql::Cluster, explain);
		REGISTER_JSON_CLASS_FIELD(mysql::Cluster, binlog);
		REGISTER_JSON_CLASS_FIELD(mysql::Cluster, conn_count);
		REGISTER_JSON_CLASS_MUST_FIELD(mysql::Cluster, address);

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
		if(!ServerConfig::Inst()->Get("mysql", this->mConfig))
		{
			return false;
		}
		if(!this->mConfig.script.empty())
		{
			std::string path = this->mConfig.script;
			return help::fs::ReadTxtFile(path, this->mConfig.script);
		}
		return true;
	}

	void MysqlDBComponent::OnStart()
	{
		if(!this->mConfig.script.empty())
		{
			this->Run(this->mConfig.script);
		}
	}

	bool MysqlDBComponent::LateAwake()
	{
		LOG_CHECK_RET_FALSE(!this->mConfig.address.empty())
		for(int x = 0; x < this->mConfig.address.size(); x++)
		{
			const std::string & address = this->mConfig.address.at(x);
			for (int index = 0; index < this->mConfig.count; index++)
			{
				mysql::Config config;
				timer::ElapsedTimer timer1;
				if(!MysqlDBComponent::DecodeUrl(address, config))
				{
					return false;
				}
				this->mRetryCount++;
				int id = (x + 1) * 100 + index + 1;
				config.script = this->mConfig.script;
				config.conn_count = this->mConfig.conn_count;
				Asio::Context& main = this->mApp->GetContext();
				tcp::Socket* tcpSocket = this->GetComponent<ThreadComponent>()->CreateSocket(config.address);
				std::shared_ptr<mysql::Client> mysqlClient = std::make_shared<mysql::Client>(id, this, config, main);
				{
					int code = mysqlClient->Start(tcpSocket);
					if (code != XCode::Ok)
					{
						LOG_ERROR("connect {} {} ", address, CodeConfig::Inst()->GetDesc(code));
						return false;
					}
					this->mClients.emplace(id, mysqlClient);
					LOG_INFO("[{}ms] connect {} ok ", timer1.GetMs(), address);
				}
			}
		}
		return true;
	}

	bool MysqlDBComponent::DecodeUrl(const std::string& address, mysql::Config& config)
	{
		if(!config.Decode(address))
		{
			return false;
		}
		std::string ip, port;
		config.Get("db", config.db);
		config.Get("password", config.password);
		LOG_CHECK_RET_FALSE(config.Get("user", config.user))
		LOG_CHECK_RET_FALSE(config.Get("address", config.address))
		return true;
	}

	void MysqlDBComponent::OnRecord(json::w::Document& document)
	{
		timer::ElapsedTimer timer1;
		this->Run(std::make_unique<mysql::Request>(mysql::cmd::PING));
		std::unique_ptr<json::w::Value> jsonObject = document.AddObject("mysql");
		{
			size_t sendByteCount = 0;
			size_t recvByteCount = 0;
			for(auto iter = this->mClients.begin(); iter != this->mClients.end(); iter++)
			{
				sendByteCount += iter->second->SendBufferBytes();
				recvByteCount += iter->second->RecvBufferBytes();
			}

			jsonObject->Add("send_memory", sendByteCount);
			jsonObject->Add("recv_memory", recvByteCount);

			jsonObject->Add("sum", this->mCount);
			jsonObject->Add("retry", this->mRetryCount);
			jsonObject->Add("client", this->mClients.size());
			jsonObject->Add("free", this->mFreeClients.Size());
			jsonObject->Add("wait", this->mMessages.size());
			jsonObject->Add("ping", fmt::format("{}ms", timer1.GetMs()));
		}
	}

	void MysqlDBComponent::OnConnectOK(int id)
	{
		this->mRetryCount--;
		if(this->mClients.find(id) != this->mClients.end())
		{
			this->AddFreeClient(id);
			LOG_DEBUG("mysql client:{} login ok", id);
		}
	}

	void MysqlDBComponent::OnDestroy()
	{
		while(!this->mMessages.empty())
		{
			this->mApp->Sleep();
			LOG_DEBUG("wait mysql request invoke => {}", this->mMessages.size());
		}
	}

	void MysqlDBComponent::OnMessage(int id, mysql::Request* request, mysql::Response* response) noexcept
	{
		this->mCount++;
		this->AddFreeClient(id);
		std::unique_ptr<mysql::Response> resp(response);
		if(response->HasError())
		{
			LOG_WARN("{}", request->ToString());
			LOG_WARN("{}", response->ToString());
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
				CONSOLE_LOG_DEBUG("[{}ms] response=>{}", request->GetCostTime(), response->ToString());
			}
		}
		int rpcId = request->GetRpcId();
		if(rpcId > 0)
		{
			this->OnResponse(rpcId, std::move(resp));
		}
	}

	void MysqlDBComponent::OnExplain(const std::string& sql, long long ms) noexcept
	{
		const std::string newSql = fmt::format("EXPLAIN {}", sql);
		std::unique_ptr<mysql::Response> response1 = this->Run(newSql);
		LoggerComponent * logger = this->GetComponent<LoggerComponent>();
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
						logger->PushLog("mysql", std::move(sqlLog));
					}
				}
			}
		}
	}

	void MysqlDBComponent::OnSendFailure(int id, mysql::Request* message)
	{
		// 重新找一个mysql client执行
		this->Send(std::unique_ptr<mysql::Request>(message));
		LOG_WARN("redis client:{} resend => {}", id, message->ToString())
	}

	void MysqlDBComponent::OnClientError(int id, int code)
	{
		auto iter = this->mClients.find(id);
		if(iter == this->mClients.end())
		{
			return;
		}
		this->mRetryCount++;
		this->mRetryClients.emplace(id);
		std::string address = iter->second->GetAddress();
		LOG_WARN("mysql client ({}) login fail", address)
	}

	void MysqlDBComponent::OnSecondUpdate(int tick) noexcept
	{
		if(this->mConfig.ping > 0 && tick % this->mConfig.ping == 0)
		{
			int id = 0;
			while(this->mFreeClients.Pop(id))
			{
				//CONSOLE_LOG_WARN("mysql client:{} ping", id)
				this->Send(id, std::make_unique<mysql::Request>(mysql::cmd::PING));
			}
		}
		if(this->mConfig.retry > 0 && tick % this->mConfig.retry == 0)
		{
			for(const int id : this->mRetryClients)
			{
				auto iter = this->mClients.find(id);
				if(iter != this->mClients.end())
				{
					iter->second->Start(nullptr);
				}
			}
			this->mRetryClients.clear();
		}
	}


	void MysqlDBComponent::Send(std::unique_ptr<mysql::Request> request, int& rpcId)
	{
		rpcId = this->BuildRpcId();
		{
			request->SetRpcId(rpcId);
			this->Send(std::move(request));
		}
	}

	void MysqlDBComponent::AddFreeClient(int id)
	{
		if(!this->mMessages.empty())
		{
			this->Send(id, std::move(this->mMessages.front()));
			this->mMessages.pop();
			return;
		}
		this->mFreeClients.Push(id);
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