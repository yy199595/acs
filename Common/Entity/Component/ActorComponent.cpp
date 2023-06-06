//
// Created by yjz on 2023/5/17.
//

#include"ActorComponent.h"
#include"Server/Config/ServerConfig.h"
#include"Lua/Component/LuaComponent.h"
namespace Tendo
{
	bool ActorComponent::LateAwake()
	{
		this->mLuaModule = nullptr;
		LuaComponent * luaComponent = this->GetComponent<LuaComponent>();
		if(luaComponent != nullptr)
		{
			this->mLuaModule = luaComponent->LoadModule(this->GetName());
			if(this->mLuaModule != nullptr)
			{
				this->mLuaModule->AddCache("AddPlayer");
				this->mLuaModule->AddCache("DelPlayer");
				this->mLuaModule->AddCache("AddServer");
				this->mLuaModule->AddCache("DelServer");
			}
		}
		return true;
	}
	bool ActorComponent::AddRandomActor(const std::string& name, Tendo::Actor* actor)
	{
		if (name.empty())
		{
			return false;
		}
		long long actorId = actor->GetEntityId();
		auto iter1 = this->mActorNames.find(name);
		if (iter1 == this->mActorNames.end())
		{
			std::vector<long long> ret;
			this->mActorNames.emplace(name, ret);
		}
		this->mActorNames[name].emplace_back(actorId);
		return true;
	}

	ServerActor* ActorComponent::Random(const std::string& name)
	{
		auto iter = this->mActorNames.find(name);
		if(iter == this->mActorNames.end())
		{
			return nullptr;
		}
		int index = rand() % (int)iter->second.size();
		return this->GetServer(iter->second[index]);
	}

	Actor* ActorComponent::GetActor(long long id)
	{
		auto iter = this->mServers.find(id);
		if(iter != this->mServers.end())
		{
			return iter->second.get();
		}
		auto iter1 = this->mPlayers.find(id);
		return iter1 != this->mPlayers.end() ? iter1->second.get() : nullptr;
	}

	bool ActorComponent::DelActor(long long id)
	{
		auto iter = this->mPlayers.find(id);
		if(iter != this->mPlayers.end())
		{
			this->mPlayers.erase(iter);
			const std::string func("DelPlayer");
			if(this->mLuaModule && this->mLuaModule->HasFunction(func))
			{
				this->mLuaModule->Call(func, id);
			}
			return true;
		}
		auto iter1 = this->mServers.find(id);
		if(iter1 != this->mServers.end())
		{
			this->mServers.erase(iter1);
			const std::string func("DelServer");
			if(this->mLuaModule && this->mLuaModule->HasFunction(func))
			{
				this->mLuaModule->Call(func, id);
			}
			return true;
		}
		return false;
	}

	bool ActorComponent::AddPlayer(std::shared_ptr<PlayerActor> player)
	{
		long long playerId = player->GetActorId();
		auto iter = this->mPlayers.find(playerId);
		if(iter != this->mPlayers.end() || !player->LateAwake())
		{
			return false;
		}
		const std::string func("AddPlayer");
		assert(player->GetActorId() > 10000);
		this->mPlayers.emplace(playerId, player);
		if(this->mLuaModule && this->mLuaModule->HasFunction(func))
		{
			this->mLuaModule->Call(func, playerId);
		}
		return true;
	}

	bool ActorComponent::AddServer(std::shared_ptr<ServerActor> server)
	{
		assert(server->GetActorId() < 10000);
		long long serverId = server->GetActorId();
		const std::string& name = server->Name();
		auto iter = this->mServers.find(serverId);
		if (name.empty() || iter != this->mServers.end() || !server->LateAwake())
		{
			return false;
		}
		std::vector<IServer*> components;
		this->mApp->GetComponents(components);
		for (IServer* listen : components)
		{
			listen->OnJoin((int)serverId);
		}
		const std::string func("AddServer");
		this->AddRandomActor(name, server.get());
		this->mServers.emplace(serverId, std::move(server));
		if(this->mLuaModule && this->mLuaModule->HasFunction(func))
		{
			this->mLuaModule->Call(func, serverId);
		}
		return true;
	}

	PlayerActor* ActorComponent::GetPlayer(long long playerId)
	{
		auto iter = this->mPlayers.find(playerId);
		return iter != this->mPlayers.end() ? iter->second.get() : nullptr;
	}

	ServerActor* ActorComponent::GetServer(long long serverId)
	{
		auto iter = this->mServers.find(serverId);
		return iter != this->mServers.end() ? iter->second.get() : nullptr;
	}

	bool ActorComponent::AddServer(std::shared_ptr<ServerActor> server, const std::string & json)
	{
		server->DecodeFromJson(json);
		return this->AddServer(server);
	}

	void ActorComponent::GetServers(std::vector<ServerActor *>& servers)
	{
		auto iter = this->mServers.begin();
		for(; iter != this->mServers.end(); iter++)
		{
			servers.emplace_back(iter->second.get());
		}
	}
}