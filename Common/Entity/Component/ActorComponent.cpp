#include"ActorComponent.h"
#include"Lua/Engine/ModuleClass.h"
#include"Entity/Lua/LuaActor.h"
#include"Util/Tools/String.h"
#include"Server/Config/ServerConfig.h"
#include"Lua/Component/LuaComponent.h"
#include"Cluster/Config/ClusterConfig.h"

namespace acs
{

	ActorComponent::ActorComponent()
	{
		this->mLuaModule = nullptr;
	}

	bool ActorComponent::LateAwake()
	{
		this->mLuaModule = nullptr;
		LuaComponent * luaComponent = this->GetComponent<LuaComponent>();
		if(luaComponent != nullptr)
		{
			const std::string & name = this->GetName();
			this->mLuaModule = luaComponent->LoadModule(name);
		}
		return this->LoadServerFromFile();
	}

	bool ActorComponent::LoadServerFromFile()
	{
		auto jsonDocument = ServerConfig::Inst()->Read("machine");
		if (jsonDocument != nullptr && jsonDocument->IsArray())
		{
			for (size_t index = 0; index < jsonDocument->MemberCount(); index++)
			{
				std::unique_ptr<json::r::Value> jsonObject;
				if (jsonDocument->Get(index, jsonObject))
				{
					Server* server = this->MakeServer(*jsonObject);
					if (server == nullptr || !this->AddServer(server))
					{
						return false;
					}
				}
			}
		}
		return true;
	}

	void ActorComponent::OnRecord(json::w::Document& document)
	{
		std::unique_ptr<json::w::Value> data = document.AddObject("actor");
		{
			data->Add("server", (int)this->mServers.size());
			data->Add("player", (int)this->mPlayers.size());
		}
	}

	void ActorComponent::OnLuaRegister(Lua::ModuleClass& luaRegister)
	{
		luaRegister.AddFunction("Send", LuaActor::Send);
		luaRegister.AddFunction("Call", LuaActor::Call);
		luaRegister.AddFunction("Publish", LuaActor::Publish);
		luaRegister.AddFunction("AddAddr", LuaPlayer::AddAddr).End("core.router");
	}

	bool ActorComponent::AddRandomActor(const std::string& name, acs::Actor* actor)
	{
		if (name.empty())
		{
			return false;
		}
		long long actorId = actor->GetId();
		auto iter1 = this->mActorNames.find(name);
		if (iter1 == this->mActorNames.end())
		{
			std::vector<long long> ret;
			this->mActorNames.emplace(name, ret);
		}
		this->mActorNames[name].emplace_back(actorId);
		return true;
	}

	Server * ActorComponent::Hash(const std::string& name, long long index)
	{
		auto iter = this->mActorNames.find(name);
		if(iter == this->mActorNames.end())
		{
			return nullptr;
		}
		size_t idx = index % iter->second.size();
		return this->GetServer(iter->second[idx]);
	}

	bool ActorComponent::HasServer(const std::string& name)
	{
		auto iter = this->mActorNames.find(name);
		if(iter == this->mActorNames.end())
		{
			return false;
		}
		return !iter->second.empty();
	}

	Server * ActorComponent::Random(const std::string& name)
	{
		auto iter = this->mActorNames.find(name);
		if(iter == this->mActorNames.end())
		{
			LOG_ERROR("not find server:{}", name);
			return nullptr;
		}
		int index = rand() % (int)iter->second.size();
		return this->GetServer(iter->second[index]);
	}

	bool ActorComponent::DelActor(long long id)
	{
		if(id == this->mApp->GetId())
		{
			LOG_WARN("try remove app:{}", id);
			return false;
		}
		auto iter = this->mPlayers.find(id);
		if(iter != this->mPlayers.end())
		{
			const std::string func("DelPlayer");
			LOG_WARN("delete player id : {}", id);
			if(this->mLuaModule && this->mLuaModule->HasFunction(func))
			{
				this->mLuaModule->Call(func, id);
			}
			delete iter->second;
			this->mPlayers.erase(iter);
			return true;
		}
		auto iter1 = this->mServers.find(id);
		if(iter1 != this->mServers.end())
		{
			const std::string func("DelServer");
			LOG_WARN("delete server id : {}", id);
			if(this->mLuaModule && this->mLuaModule->HasFunction(func))
			{
				this->mLuaModule->Call(func, id);
			}
			delete iter1->second;
			this->mServers.erase(iter1);
			return true;
		}
		return false;
	}

	bool ActorComponent::UpdateServer(json::r::Value& document)
	{
		int id = 0;
		std::string name;
		LOG_CHECK_RET_FALSE(document.Get("id", id));
		LOG_CHECK_RET_FALSE(document.Get("name", name));
		Server * server = this->GetServer(id);
		if(server == nullptr)
		{
			return false;
		}
		bool result = false;
		LOG_CHECK_RET_FALSE(name == server->Name());
		std::unique_ptr<json::r::Value> listenData;
		if(document.Get("listen", listenData))
		{
			std::vector<const char * > keys;
			if(listenData->GetKeys(keys) > 0)
			{
				std::unique_ptr<json::r::Value> value;
				for (const char* key: keys)
				{
					listenData->Get(key, value);
					if(server->AddListen(key, value->ToString()))
					{
						result = true;
					}
				}
			}
		}
		return result;
	}

	Server * ActorComponent::MakeServer(const json::r::Value & document)
	{
		int id = 0;
		std::string name;
		LOG_CHECK_RET_NULL(document.Get("id", id));
		std::unique_ptr<json::r::Value> listenData;
		LOG_CHECK_RET_NULL(document.Get("name", name));
		LOG_CHECK_RET_NULL(document.Get("listen", listenData));
		const NodeConfig* nodeConfig = ClusterConfig::Inst()->GetConfig(name);
		if (nodeConfig == nullptr)
		{
			LOG_ERROR("not find ClusterConfig : {}", name);
			return nullptr;
		}

		Server* server = this->GetServer(id);
		if (server != nullptr)
		{
			return server;
		}
		server = new Server(id, name);
		std::vector<const char*> keys;
		if (listenData->GetKeys(keys) > 0)
		{
			std::unique_ptr<json::r::Value> value;
			for (const char* key: keys)
			{
				listenData->Get(key, value);
				server->AddListen(key, value->ToString());
			}
		}
		return server;
	}

	bool ActorComponent::AddPlayer(Player * player)
	{
		long long playerId = player->GetId();
		if(this->GetPlayer(playerId) != nullptr)
		{
			return false;
		}
		LOG_CHECK_RET_FALSE(player->LateAwake());
		const static std::string func("AddPlayer");
		if(this->mLuaModule && this->mLuaModule->HasFunction(func))
		{
			this->mLuaModule->Call(func, playerId);
		}
		this->mPlayers.emplace(playerId, player);
		return true;
	}

	bool ActorComponent::AddServer(Server * server)
	{
		if(server == nullptr) return false;
		int serverId = server->GetSrvId();
		if(this->GetServer(serverId) != nullptr)
		{
			return false;
		}

		LOG_CHECK_RET_FALSE(server->LateAwake());
		{
			std::string json;
			server->EncodeToJson(&json);
			LOG_DEBUG("add new server : {}", json);
		}

		std::vector<IServer*> components;
		this->mApp->GetComponents(components);
		for (IServer* listen : components)
		{
			listen->OnJoin((int)serverId);
		}
		const std::string func("AddServer");
		this->AddRandomActor(server->Name(), server);
		if(this->mLuaModule && this->mLuaModule->HasFunction(func))
		{
			this->mLuaModule->Call(func, serverId);
		}
		this->mServers.emplace(serverId, server);
		return true;
	}

	size_t ActorComponent::GetServers(std::vector<int>& servers)
	{
		auto iter = this->mServers.begin();
		servers.reserve(this->mServers.size());
		for(; iter != this->mServers.end(); iter++)
		{
			const Server * server = iter->second;
			if(ClusterConfig::Inst()->GetConfig(server->Name()) != nullptr)
			{
				servers.emplace_back(server->GetSrvId());
			}
		}
		return servers.size();
	}

	size_t ActorComponent::GetServers(const std::string& name, std::vector<int>& servers)
	{
		auto iter = this->mActorNames.find(name);
		if(iter == this->mActorNames.end())
		{
			return 0;
		}
		for(long long id : iter->second)
		{
			servers.emplace_back((int)id);
		}
		return servers.size();
	}

	bool ActorComponent::GetListen(int id, const std::string& name, std::string& addr)
	{
		auto iter = this->mServers.find(id);
		if(iter == this->mServers.end())
		{
			return false;
		}
		return iter->second->GetListen(name, addr);
	}

	int ActorComponent::Broadcast(const std::string& func)
	{
		int count = 0;
		auto iter = this->mServers.begin();
		for(; iter != this->mServers.end(); iter++)
		{
			Server * server = iter->second;
			if(server->Send(func) == XCode::Ok)
			{
				count++;
			}
		}
		return count;
	}

	int ActorComponent::Broadcast(const std::string& func, const pb::Message& message)
	{
		int count = 0;
		auto iter = this->mServers.begin();
		for(; iter != this->mServers.end(); iter++)
		{
			//if(!this->mApp->Equal(iter->first))
			{
				Server * server = iter->second;
				if(server->Send(func, message) == XCode::Ok)
				{
					count++;
				}
			}
		}
		return count;
	}

	bool ActorComponent::HasPlayer(long long id) const
	{
		auto iter = this->mPlayers.find(id);
		return iter != this->mPlayers.end();
	}

	bool ActorComponent::HasServer(long long id) const
	{
		auto iter = this->mServers.find(id);
		return iter != this->mServers.end();
	}

	bool ActorComponent::HasServer(const std::string& name) const
	{
		auto iter = this->mActorNames.find(name);
		if(iter == this->mActorNames.end())
		{
			return false;
		}
		return !iter->second.empty();
	}

	Actor* ActorComponent::GetActor(long long id)
	{
		auto iter = this->mServers.find(id);
		if(iter != this->mServers.end())
		{
			return iter->second;
		}
		auto iter1 = this->mPlayers.find(id);
		return iter1 != this->mPlayers.end() ? iter1->second : nullptr;
	}

	Server* ActorComponent::GetServer(long long serverId)
	{
		auto iter1 = this->mServers.find(serverId);
		return iter1 != this->mServers.end() ? iter1->second : nullptr;
	}

	Player* ActorComponent::GetPlayer(long long playerId)
	{
		auto iter1 = this->mPlayers.find(playerId);
		return iter1 != this->mPlayers.end() ? iter1->second : nullptr;
	}
}