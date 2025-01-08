//
// Created by yjz on 2023/5/25.
//

#include"Master.h"
#include"Entity/Actor/App.h"
#include"Util/Tools/TimeHelper.h"
#include"Cluster/Config/ClusterConfig.h"
#include"Redis/Component/RedisComponent.h"
namespace acs
{
	constexpr int EXP_TIME = 15;
	Master::Master() : mActorComponent(nullptr) { }
	
	bool Master::OnInit()
	{
		BIND_COMMON_HTTP_METHOD(Master::Del);
		BIND_COMMON_HTTP_METHOD(Master::Push);
		BIND_COMMON_HTTP_METHOD(Master::Find);
		this->mRedis = this->GetComponent<RedisComponent>();
		ClusterConfig::Inst()->GetServers(this->mConfServers);
		this->mActorComponent = this->GetComponent<ActorComponent>();
		this->mLock  = std::make_unique<CoroutineLock>(App::Coroutine());
		return this->mRedis != nullptr && this->mActorComponent != nullptr;
	}

	void Master::OnSecondUpdate(int tick) noexcept
	{
		if (tick % 10 != 0) return;

		for (const std::string& name: this->mConfServers)
		{
			if (!this->mActorComponent->HasServer(name))
			{
				LOG_WARN("===== wait {} register =====", name);
				return;
			}
		}

		std::vector<int> servers;
		this->mActorComponent->GetServers(servers);
		for(const int serverId : servers)
		{
			this->mApp->StartCoroutine([this, serverId]()
			{
				const std::string func("NodeSystem.Ping");
				Server* server = this->mActorComponent->GetServer(serverId);
				if (server == nullptr || server->Call(func) != XCode::Ok)
				{
					this->OnDisConnect(serverId);
					LOG_ERROR("ping [{}:{}] ", server->Name(), serverId);
					return;
				}
				std::string json;
				if(this->mServerData.Get(serverId, json))
				{
					std::string key = fmt::format("server:{}", serverId);
					this->mRedis->Run("SET", key, json, "EX", EXP_TIME);
				}
			});
		}
	}

	int Master::Find(const http::FromContent & request, json::w::Document & response)
	{
		int id = 0;
		int appId = 0;
		request.Get("id", id);
		LOG_CHECK_RET_FALSE(request.Get("app", appId));
		Server * server = this->mActorComponent->GetServer(appId);
		if(server == nullptr)
		{
			return XCode::NotFoundActor;
		}
		std::string json;
		com::type::json message;
		if(this->mServerData.Get(id, json))
		{
			message.set_json(json);
			return server->Send("NodeSystem.Add", message);
		}
		this->mLock->Lock();
		auto iter = this->mServerData.Begin();
		for(; iter != this->mServerData.End(); iter++)
		{
			message.set_json(iter->second);
			int code = server->Send("NodeSystem.Add", message);
			if(code != XCode::Ok)
			{
				this->mLock->UnLock();
				return code;
			}
		}
		this->mLock->UnLock();
		return XCode::Ok;
	}

	int Master::Push(const json::r::Document & request, json::w::Value & res)
	{
		int id = 0;
		LOG_ERROR_CHECK_ARGS(request.Get("id", id));
		Server * server = this->mActorComponent->MakeServer(request);
		if(server == nullptr)
		{
			return XCode::Failure;
		}
		this->mLock->Lock();
		com::type::json message;
		message.set_json(request.ToString());

		// 通知其他服务器
		std::vector<int> servers;
		this->mActorComponent->GetServers(servers);
		std::string key = fmt::format("server:{}", id);
		this->mRedis->Run("SET", key, message.json(), "EX", EXP_TIME);
		for(const int serverId : servers)
		{
			Server * logicServer = this->mActorComponent->GetServer(serverId);
			if(logicServer != nullptr)
			{
				logicServer->Call("NodeSystem.Add", message);
			}
		}
		this->mLock->UnLock();
		this->mServerData.Set(id, message.json());
		LOG_DEBUG("[server.{}] register ok : {}", server->Name(), request.ToString());
		return XCode::Ok;
	}

	int Master::Del(const http::FromContent & request, json::w::Value & res)
	{
		int actorId = 0;
		request.Get("id", actorId);
		if (!this->mActorComponent->DelActor(actorId))
		{
			return XCode::NotFoundActor;
		}
		this->mLock->Lock();
		this->mServerData.Del(actorId);

		std::vector<int> servers;
		this->mActorComponent->GetServers(servers);
		for(const int serverId : servers)
		{
			com::type::int64 message;
			message.set_value(actorId);
			Server * logicServer = this->mActorComponent->GetServer(serverId);
			if(logicServer != nullptr && logicServer->Call("NodeSystem.Del", message) == XCode::Ok)
			{
				LOG_DEBUG("notice server({}) ok", serverId);
			}
		}
		LOG_WARN("del server actor id : {}", actorId);
		this->mLock->UnLock();
		return XCode::Ok;
	}

	void Master::OnDisConnect(int serverId)
	{
		com::type::int64 request;
		request.set_value(serverId);
		this->mServerData.Del(serverId);
		this->mApp->Send("NodeSystem.Del", request);
		this->mRedis->Run("DEL", fmt::format("server:", serverId));
	}
}