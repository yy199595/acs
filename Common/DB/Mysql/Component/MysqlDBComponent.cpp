//
// Created by zmhy0073 on 2022/7/16.
//

#include "MysqlDBComponent.h"
#include "Entity/Actor/App.h"
#include "Server/Config/ServerConfig.h"
#include "Server/Component/ThreadComponent.h"
#include "Timer/Timer/ElapsedTimer.h"
#include "Message/s2s/registry.pb.h"
#include "Util/Sql/SqlHelper.h"
#include "Util/Tools/Random.h"
#include "Lua/Lib/Lib.h"
#include "Util/File/FileHelper.h"

namespace acs
{
	MysqlTask::MysqlTask(int taskId)
		: IRpcTask<mysql::Response>(taskId)
	{

	}
}

namespace acs
{
	MysqlDBComponent::MysqlDBComponent()
	{
		REGISTER_JSON_CLASS_FIELD(mysql::Config, db);
		REGISTER_JSON_CLASS_FIELD(mysql::Config, user);
		REGISTER_JSON_CLASS_FIELD(mysql::Config, ping);
		REGISTER_JSON_CLASS_FIELD(mysql::Config, count);
		REGISTER_JSON_CLASS_FIELD(mysql::Config, passwd);
		REGISTER_JSON_CLASS_FIELD(mysql::Config, script);
		REGISTER_JSON_CLASS_MUST_FIELD(mysql::Config, address);
	}

	bool MysqlDBComponent::Awake()
	{
		this->mConfig.count = 1;
		LuaCCModuleRegister::Add([](Lua::CCModule & ccModule) {
			ccModule.Open("db.mysql", lua::lib::luaopen_lmysqldb);
		});
		return ServerConfig::Inst()->Get("mysql", this->mConfig);
	}

	bool MysqlDBComponent::LateAwake()
	{
		ThreadComponent * threadComponent = this->GetComponent<ThreadComponent>();
		for(int index = 0; index < this->mConfig.count; index++)
		{
			int id = index + 1;
			timer::ElapsedTimer timer1;
			Asio::Context & main = this->mApp->GetContext();
			tcp::Socket * tcpSocket = threadComponent->CreateSocket(this->mConfig.address);
			std::shared_ptr<mysql::Client> mysqlClient = std::make_shared<mysql::Client>(id, tcpSocket, this, this->mConfig, main);
			if(!mysqlClient->Connect())
			{
				LOG_ERROR("connect mysql [{}] fail", this->mConfig.address)
				return false;
			}
			this->mFreeClients.Push(id);
			this->mClients.emplace(id, mysqlClient);
			LOG_INFO("[{}ms] connect mysql [{}] ok", timer1.GetMs(), this->mConfig.address)
		}
		return true;
	}

	void MysqlDBComponent::OnMessage(int id, mysql::Request* request, mysql::Response* response) noexcept
	{
		if(this->mMessages.empty())
		{
			this->mFreeClients.Push(id);
		}
		else
		{
			std::unique_ptr<mysql::Request>& req = this->mMessages.front();
			{
				this->Send(id, std::move(req));
				this->mMessages.front();
			}
		}
		if(response->GetPackageCode() == mysql::PACKAGE_ERR)
		{
			LOG_ERROR("{}", request->ToString());
			LOG_ERROR("{}", response->GetBuffer());
		}
		else
		{
//			const mysql::Result & result = response->GetResult();
//			for(const std::string & json : result.contents)
//			{
//				jsonArray2->PushJson(json);
//			}
			//LOG_DEBUG("({})[{}ms] {}", result.contents.size(), request->GetCostTime(), request->ToString());
//			LOG_INFO("result => {}", document.JsonString(true))
		}
		int rpcId = request->GetRpcId();
		this->OnResponse(rpcId, std::unique_ptr<mysql::Response>(response));
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