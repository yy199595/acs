//
// Created by zmhy0073 on 2022/7/16.
//
#ifdef __ENABLE_MYSQL__


#include"MysqlDBComponent.h"
#include"Entity/Actor/App.h"
#include"Util/Sql/SqlHelper.h"
#include"Mysql/Client/MysqlClient.h"
#include"Mysql/Lua/LuaMysql.h"
#include"Lua/Engine/ModuleClass.h"
#include"Timer/Timer/ElapsedTimer.h"
#include"Proto/Component/ProtoComponent.h"
namespace joke
{
    MysqlTask::MysqlTask(int taskId)
        : IRpcTask<Mysql::Response>(taskId), mMessage(nullptr)
    {

    }
}

namespace joke
{
	bool MysqlDBComponent::Awake()
	{
		std::unique_ptr<json::r::Value> mysqlObject;
		if(!ServerConfig::Inst()->Get("mysql", mysqlObject))
		{
			return false;
		}
		LOG_CHECK_RET_FALSE(mysqlObject->Get("db", this->mConfig.DB))
		LOG_CHECK_RET_FALSE(mysqlObject->Get("user", this->mConfig.User))
		LOG_CHECK_RET_FALSE(mysqlObject->Get("ping", this->mConfig.Ping))
		LOG_CHECK_RET_FALSE(mysqlObject->Get("count", this->mConfig.MaxCount))
		LOG_CHECK_RET_FALSE(mysqlObject->Get("passwd", this->mConfig.Password))
		LOG_CHECK_RET_FALSE(mysqlObject->Get("address", this->mConfig.Address))
		std::unique_ptr<json::r::Value> mysqlArray;
		if(mysqlObject->Get("table", mysqlArray) && mysqlArray->IsArray())
		{
			for(size_t index = 0; index < mysqlArray->MemberCount(); index++)
			{
				std::string proto;
				if(mysqlArray->Get(index, proto))
				{
					this->mConfig.Tables.emplace_back(proto);
				}
			}
		}
		return true;
	}

	bool MysqlDBComponent::LateAwake()
	{
#ifdef __DEBUG__
		this->mConfig.MaxCount = 1;
#endif
		MysqlClient * mysqlClient = nullptr;
		for (int index = 0; index < this->mConfig.MaxCount; index++)
		{
			timer::ElapsedTimer timer1;
			mysqlClient = new MysqlClient(this, this->mConfig);
			{
				LOG_CHECK_RET_FALSE(mysqlClient->Start());
				const std::string & address = this->mConfig.Address;
				LOG_INFO("[{}ms] connect mysql [{}] ok", timer1.GetMs(), address);

			}
			this->mMysqlClients.Push(mysqlClient);
		}
		ProtoComponent * proto = this->mApp->GetProto();
		for(const std::string & path : this->mConfig.Tables)
		{
			std::vector<std::string> types;
			proto->Import(path.c_str(), types);
		}
		return this->mConfig.MaxCount > 0;
	}

	void MysqlDBComponent::OnDestroy()
	{
		MysqlClient * mysqlClient = nullptr;
		while(this->mMysqlClients.Pop(mysqlClient))
		{
			mysqlClient->Stop();
		}
	}

	void MysqlDBComponent::OnMessage(Mysql::IRequest * request, Mysql::Response * message)
	{
		if(message != nullptr)
		{
			if(!message->GetError().empty())
			{
				std::string sql;
				request->GetSql(sql);
				LOG_ERROR("mysql request : {}", sql);
				const std::string & err = message->GetError();
				LOG_ERROR("[{}ms]mysql response : {}", request->GetMs(), err);
			}
		}
		int key = request->GetRpcId();
		this->OnResponse(key, message);
		delete request;
	}

	void MysqlDBComponent::OnLuaRegister(Lua::ModuleClass& luaRegister)
	{
		luaRegister.AddFunction("Exec", Lua::LuaMysql::Exec);
		luaRegister.AddFunction("Find", Lua::LuaMysql::Query);
		luaRegister.AddFunction("FindOnce", Lua::LuaMysql::QueryOnce);
		luaRegister.AddFunction("CreateTable", Lua::LuaMysql::CreateTable);
		luaRegister.End("db.mysql");
	}


	Mysql::Response * MysqlDBComponent::Run(std::unique_ptr<Mysql::IRequest> command)
	{
		int rpcId = 0;
		if (!this->Send(std::move(command), rpcId))
		{
			return nullptr;
		}
		return this->AddTask(rpcId, new MysqlTask(rpcId))->Await();
	}

	std::unique_ptr<Mysql::Response> MysqlDBComponent::SyncRun(std::unique_ptr<Mysql::IRequest> command)
	{
		MysqlClient * mysqlClient = nullptr;
		if(!this->mMysqlClients.Pop(mysqlClient))
		{
			return nullptr;
		}
		this->mMysqlClients.Push(mysqlClient);
		return mysqlClient->Sync(std::move(command));
	}

	bool MysqlDBComponent::Send(std::unique_ptr<Mysql::IRequest> command, int & rpcId)
	{
		MysqlClient * mysqlClient = nullptr;
		command->SetRpcId(this->mNumPool.BuildNumber());
		if (!this->mMysqlClients.Pop(mysqlClient))
		{
			return false;
		}
		rpcId = command->GetRpcId();
		mysqlClient->Push(std::move(command));
		this->mMysqlClients.Push(mysqlClient);
		return true;
	}

	bool MysqlDBComponent::Execute(std::unique_ptr<Mysql::IRequest> command)
	{
		Mysql::Response * response = this->Run(std::move(command));
		return response != nullptr && response->IsOk();
	}

	bool MysqlDBComponent::SyncExecute(std::unique_ptr<Mysql::IRequest> command)
	{
		std::unique_ptr<Mysql::Response> response = this->SyncRun(std::move(command));
		return response != nullptr && response->IsOk();
	}
}

#endif