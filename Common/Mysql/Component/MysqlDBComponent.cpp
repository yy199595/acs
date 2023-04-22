//
// Created by zmhy0073 on 2022/7/16.
//
#ifdef __ENABLE_MYSQL__


#include"MysqlDBComponent.h"
#include"Entity/Unit/App.h"
#include"Util/Sql/SqlHelper.h"
#include"Mysql/Client/MysqlClient.h"
#include"Mysql/Lua/LuaMysql.h"
#include"Lua/Engine/ClassProxyHelper.h"
namespace Tendo
{
    MysqlTask::MysqlTask(int taskId)
        : IRpcTask<Mysql::Response>(taskId)
    {

    }

    void MysqlTask::OnResponse(std::shared_ptr<Mysql::Response> response)
    {
        this->mTask.SetResult(response);
    }
}

namespace Tendo
{

}

namespace Tendo
{
	bool MysqlDBComponent::Awake()
	{
		std::string path;
		this->mSqlHelper = std::make_unique<SqlHelper>();
		const ServerConfig * config = ServerConfig::Inst();
		LOG_CHECK_RET_FALSE(config->GetPath("db", path));
		LOG_CHECK_RET_FALSE(this->mConfig.LoadConfig(path));
		this->mMainClient = std::make_shared<MysqlClient>(this, this->mConfig);
		return this->mMainClient->StartConnect();
	}

	bool MysqlDBComponent::LateAwake()
	{
        int id = 0;
		for (int index = 0; index < this->mConfig.MaxCount; index++)
		{
            id++;
			std::shared_ptr<MysqlClient> mysqlClient
				= std::make_shared<MysqlClient>(this, this->mConfig);

			mysqlClient->Start();
			this->mAllotQueue.push(id);
			this->mMysqlClients.emplace(id, std::move(mysqlClient));
		}
		return !this->mMysqlClients.empty();
	}

	bool MysqlDBComponent::GetClientHandle(int & id)
	{
		if (this->mAllotQueue.empty())
		{
			return false;
		}
		id = this->mAllotQueue.front();
		this->mAllotQueue.push(id);
		this->mAllotQueue.pop();
		return true;
	}

	void MysqlDBComponent::OnDestroy()
	{
		while (!this->mAllotQueue.empty())
		{
			this->mAllotQueue.pop();
		}
		auto iter = this->mMysqlClients.begin();
		for (;iter != this->mMysqlClients.end(); iter++)
		{
			iter->second->Stop();
		}
		this->mMysqlClients.clear();
	}

	void MysqlDBComponent::OnSecondUpdate(int tick)
	{
		if (tick % this->mConfig.Ping)
		{
			auto iter = this->mMysqlClients.begin();
			long long now = Helper::Time::NowSecTime();
			for (; iter != this->mMysqlClients.end(); iter++)
			{
				std::shared_ptr<MysqlClient>& mysqlClient = iter->second;
				if (mysqlClient->LastRunTime() != 0 && now - mysqlClient->LastRunTime() >= this->mConfig.Ping)
				{
					mysqlClient->Ping();
#ifdef __DEBUG__
					//CONSOLE_LOG_DEBUG("start ping mysql client : " << iter->first);
#endif
				}
			}
		}
	}

	void MysqlDBComponent::OnConnectSuccessful(const std::string& address)
	{
		LOG_INFO("mysql client [" << address << "] auth successful");
	}

	void MysqlDBComponent::OnMessage(std::shared_ptr<Mysql::Response> message)
	{
		if(!message->IsOk())
		{
			//LOG_ERROR("mysql error : " << message->GetError());
		}
		int key = message->TaskId();
		this->OnResponse(key, message);
	}

    bool MysqlDBComponent::Ping(int index)
    {
        std::shared_ptr<Mysql::PingCommand> command
            = std::make_shared<Mysql::PingCommand>();
		return this->Run(index, command)->IsOk();
    }

	void MysqlDBComponent::OnLuaRegister(Lua::ClassProxyHelper& luaRegister)
	{
		luaRegister.BeginNewTable("Mysql");
		luaRegister.PushExtensionFunction("Make", Lua::LuaMysql::Make);
		luaRegister.PushExtensionFunction("Exec", Lua::LuaMysql::Exec);
		luaRegister.PushExtensionFunction("Query", Lua::LuaMysql::Query);
		luaRegister.PushExtensionFunction("QueryOnce", Lua::LuaMysql::QueryOnce);
		luaRegister.PushExtensionFunction("CreateTable", Lua::LuaMysql::CreateTable);

	}

	std::shared_ptr<Mysql::Response> MysqlDBComponent::Run(int index, const std::shared_ptr<Mysql::ICommand>& command)
	{
		int rpcId = 0;
		if (!this->Send(index, command, rpcId))
		{
			return nullptr;
		}
		std::shared_ptr<MysqlTask> mysqlTask = std::make_shared<MysqlTask>(rpcId);
		std::shared_ptr<Mysql::Response> response = this->AddTask(rpcId, mysqlTask)->Await();
		if (response != nullptr && !response->IsOk())
		{
			LOG_ERROR(response->GetError());
		}
		return response;
	}

	std::shared_ptr<Mysql::Response> MysqlDBComponent::Run(const std::shared_ptr<Mysql::ICommand>& command)
	{
		return this->mMainClient->Run(command);
	}

	bool MysqlDBComponent::Send(int id,  const std::shared_ptr<Mysql::ICommand> & command, int & rpcId)
	{
		auto iter = this->mMysqlClients.find(id);
		if(iter == this->mMysqlClients.end())
		{
			return false;
		}
		rpcId = this->PopTaskId();
		command->SetRpcId(rpcId);
		iter->second->Push(command);
		return true;
	}

	bool MysqlDBComponent::Execute(int index, const std::shared_ptr<Mysql::ICommand>& command)
	{
		std::shared_ptr<Mysql::Response> response = this->Run(index, command);
		return response != nullptr && response->IsOk();
	}

	bool MysqlDBComponent::Execute(const std::shared_ptr<Mysql::ICommand>& command)
	{
		std::shared_ptr<Mysql::Response> response = this->Run(command);
		return response != nullptr && response->IsOk();
	}
}

#endif