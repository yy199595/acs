//
// Created by 64658 on 2025/2/18.
//

#include "XCode/XCode.h"
#include "Lua/Lib/Lib.h"
#include "Entity/Actor/App.h"
#include "PgsqlDBComponent.h"
#include "Util/File/FileHelper.h"
#include "Server/Config/CodeConfig.h"
#include "Server/Config/ServerConfig.h"
#include "Server/Component/ThreadComponent.h"

#include "Timer/Timer/ElapsedTimer.h"
#include "Log/Component/LoggerComponent.h"

namespace acs
{
	PgsqlDBComponent::PgsqlDBComponent()
	{
		this->mSumCount = 0;
		this->mRetryCount = 0;
		this->mThread = nullptr;
		this->mConfig.count = 1;
		this->mConfig.debug = false;

		REGISTER_JSON_CLASS_FIELD(db::Explain, open);
		REGISTER_JSON_CLASS_FIELD(db::Explain, command);
		REGISTER_JSON_CLASS_FIELD(pgsql::Cluster, ping);
		REGISTER_JSON_CLASS_FIELD(pgsql::Cluster, count);
		REGISTER_JSON_CLASS_FIELD(pgsql::Cluster, retry);
		REGISTER_JSON_CLASS_FIELD(pgsql::Cluster, debug);
		REGISTER_JSON_CLASS_FIELD(pgsql::Cluster, script);
		REGISTER_JSON_CLASS_FIELD(pgsql::Cluster, explain);
		REGISTER_JSON_CLASS_FIELD(pgsql::Cluster, conn_count);
		REGISTER_JSON_CLASS_MUST_FIELD(pgsql::Cluster, address);
	}

	bool PgsqlDBComponent::Awake()
	{
		LuaCCModuleRegister::Add([](Lua::CCModule& ccModule)
		{
			ccModule.Open("db.pgsql", lua::lib::luaopen_lpgsqldb);
		});
		LOG_CHECK_RET_FALSE(ServerConfig::Inst()->Get("pgsql", this->mConfig))
		if(!this->mConfig.script.empty())
		{
			std::string path = this->mConfig.script;
			return help::fs::ReadTxtFile(path, this->mConfig.script);
		}
		return true;
	}

	bool PgsqlDBComponent::LateAwake()
	{
		LOG_CHECK_RET_FALSE(!this->mConfig.address.empty())
		for (int x = 0; x < this->mConfig.address.size(); x++)
		{
			const std::string& address = this->mConfig.address[x];
			for (int index = 0; index < this->mConfig.count; index++)
			{
				pgsql::Config config;
				timer::ElapsedTimer timer1;
				if (!PgsqlDBComponent::DecodeUrl(address, config))
				{
					return false;
				}
				this->mRetryCount++;
				int id = (x + 1) * 100 + index + 1;
				config.script = this->mConfig.script;
				config.conn_count = this->mConfig.conn_count;
				Asio::Context& main = this->mApp->GetContext();
				tcp::Socket* tcpSocket = this->GetComponent<ThreadComponent>()->CreateSocket(config.address);
				std::shared_ptr<pgsql::Client> client = std::make_shared<pgsql::Client>(id, this, config, main);
				{
					int code = client->Start(tcpSocket);
					if (code != XCode::Ok)
					{
						std::string desc = CodeConfig::Inst()->GetDesc(code);
						LOG_ERROR("connect {} => {} ", address, desc);
						return false;
					}
					this->mClients.emplace(id, client);
					LOG_INFO("[{}ms] connect {} ok", timer1.GetMs(), address);
				}
			}
		}
		return true;
	}

	void PgsqlDBComponent::OnStart()
	{
		if(!this->mConfig.script.empty())
		{
			this->Run(this->mConfig.script);
		}
	}

	void PgsqlDBComponent::OnDestroy()
	{
		while(!this->mMessages.empty())
		{
			this->mApp->Sleep();
			LOG_DEBUG("wait pgsql request invoke => {}", this->mMessages.size());
		}
	}

	bool PgsqlDBComponent::DecodeUrl(const std::string& url, pgsql::Config& config)
	{
		if (!config.Decode(url))
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

	void PgsqlDBComponent::OnConnectOK(int id)
	{
		this->mRetryCount--;
		if (this->mClients.find(id) != this->mClients.end())
		{
			this->AddFreeClient(id);
			LOG_DEBUG("pgsql client:{} login ok", id);
		}
	}

	void PgsqlDBComponent::OnClientError(int id, int code)
	{
		auto iter = this->mClients.find(id);
		if (iter == this->mClients.end())
		{
			return;
		}
		this->mRetryCount++;
		this->mRetryClients.emplace(id);
		std::string address = iter->second->GetAddress();
		LOG_WARN("pgsql client ({}) login fail", address)
	}

	void PgsqlDBComponent::OnSecondUpdate(int tick) noexcept
	{
		if (this->mConfig.ping > 0 && tick % this->mConfig.ping == 0)
		{
			int id = 0;
			std::string sql("SELECT 1");
			while (this->mFreeClients.Pop(id))
			{
				//CONSOLE_LOG_WARN("pgsql client:{} ping", id)
				this->Send(id, std::make_unique<pgsql::Request>(sql));
			}
		}
		if (this->mConfig.retry > 0 && tick % this->mConfig.retry == 0)
		{
			for (const int id: this->mRetryClients)
			{
				auto iter = this->mClients.find(id);
				if (iter != this->mClients.end())
				{
					iter->second->Start(nullptr);
				}
			}
			this->mRetryClients.clear();
		}
	}

	void PgsqlDBComponent::OnRecord(json::w::Document& document)
	{
		timer::ElapsedTimer timer1;
		this->Run("SELECT 1");
		std::unique_ptr<json::w::Value> jsonValue = document.AddObject("pgsql");
		{
			jsonValue->Add("sum", this->mSumCount);
			jsonValue->Add("retry", this->mRetryCount);
			jsonValue->Add("client", this->mClients.size());
			jsonValue->Add("free", this->mFreeClients.Size());
			jsonValue->Add("ping", fmt::format("{}ms", timer1.GetMs()));
			jsonValue->Add("wait", this->mMessages.size() + this->AwaitCount());
		}
	}

	void PgsqlDBComponent::OnSendFailure(int id, pgsql::Request* message)
	{
		this->Send(std::unique_ptr<pgsql::Request>(message));
		LOG_WARN("[{}] resend pgsql => {}", id, message->ToString())
	}

	void PgsqlDBComponent::OnExplain(const std::string& sql, long long ms) noexcept
	{
		const std::string newSql = fmt::format("EXPLAIN {}", sql);
		std::unique_ptr<pgsql::Response> response1 = this->Run(newSql);
		LoggerComponent* logger = this->GetComponent<LoggerComponent>();
		{
			if (!response1->mResults.empty())
			{
				const std::string& result = response1->mResults.front();
				std::unique_ptr<custom::LogInfo> sqlLog = std::make_unique<custom::LogInfo>();
				{
					sqlLog->Level = custom::LogLevel::None;
					sqlLog->Content = fmt::format("[{}ms] {}\n", ms, sql);
					{
						sqlLog->Content.append(result);
						logger->PushLog("pgsql", std::move(sqlLog));
					}
				}
			}
		}
	}

	void PgsqlDBComponent::OnMessage(int id, pgsql::Request* request, pgsql::Response* response) noexcept
	{
		this->mSumCount++;
		this->AddFreeClient(id);
		int rpcId = request->GetRpcId();
		std::unique_ptr<pgsql::Response> resp(response);
		if (!response->mError.empty())
		{
			LOG_WARN("[request] {}", request->ToString());
			LOG_WARN("[response] {}", response->mError);
		}
		else
		{
			if (this->mConfig.explain.open && rpcId > 0)
			{
				std::string cmd;
				if (request->GetCommand(cmd) && cmd != "EXPLAIN" && this->mConfig.explain.HasCommand(cmd))
				{
					long long ms = request->GetCostTime();
					std::string sql = request->ToString();
					this->mApp->StartCoroutine([sql, this, ms]
					{
						this->OnExplain(sql, ms);
					});
				}
			}

			if (this->mConfig.debug)
			{
				LOG_DEBUG("[{}ms] ({}) {}", request->GetCostTime(), response->mResults.size(), request->ToString())
			}
		}
		if (rpcId > 0)
		{
			this->OnResponse(rpcId, std::move(resp));
		}
	}

	std::unique_ptr<pgsql::Response> PgsqlDBComponent::Run(const std::string& sql)
	{
		return this->Run(std::make_unique<pgsql::Request>(sql));
	}

	std::unique_ptr<pgsql::Response> PgsqlDBComponent::Run(std::unique_ptr<pgsql::Request> request)
	{
		int rpcId = 0;
		this->Send(std::move(request), rpcId);
		return this->BuildRpcTask<PgsqlTask>(rpcId)->Await();
	}

	void PgsqlDBComponent::Send(std::unique_ptr<pgsql::Request> request)
	{
		int rpcId = 0;
		this->Send(std::move(request), rpcId);
	}

	void PgsqlDBComponent::AddFreeClient(int id)
	{
		if(!this->mMessages.empty())
		{
			this->Send(id, std::move(this->mMessages.front()));
			this->mMessages.pop();
			return;
		}
		this->mFreeClients.Push(id);
	}

	void PgsqlDBComponent::Send(std::unique_ptr<pgsql::Request> request, int& rpcId)
	{
		int id = 0;
		rpcId = this->BuildRpcId();
		request->SetRpcId(rpcId);
		if (!this->mFreeClients.Pop(id))
		{
			this->mMessages.emplace(std::move(request));
			return;
		}
		this->Send(id, std::move(request));
	}

	void PgsqlDBComponent::Send(int id, std::unique_ptr<pgsql::Request> request)
	{
		auto iter = this->mClients.find(id);
		if (iter != this->mClients.end())
		{
			iter->second->Send(std::move(request));
			return;
		}
		LOG_ERROR("not find client:{} {}", id, request->ToString())
	}
}