#include"ActorComponent.h"
#include"Lua/Engine/ModuleClass.h"
#include"Server/Config/ServerConfig.h"
#include"Lua/Component/LuaComponent.h"
#include "Core/Event/IEvent.h"
#include "Util/Tools/Random.h"

namespace actor
{
	bool Group::AddItem(long long id)
	{
		for(const long long & item : this->mItems)
		{
			if(item == id)
			{
				return false;
			}
		}
		this->mItems.emplace_back(id);
		return true;
	}

	bool Group::Random(long long& id)
	{
		if(this->mItems.empty())
		{
			return false;
		}
		size_t count = this->mItems.size();
		size_t index = help::Rand::Random<size_t>(0, count - 1);
		id = this->mItems.at(index);
		return true;
	}

	bool Group::Hash(long long hash, long long& id)
	{
		if(this->mItems.empty())
		{
			return false;
		}
		size_t count = this->mItems.size();
		size_t index = hash % count;
		id = this->mItems.at(index);
		return true;
	}


}

namespace acs
{

	ActorComponent::ActorComponent() = default;

	bool ActorComponent::LateAwake()
	{
		return this->LoadServerFromFile();
	}

	void ActorComponent::OnRecord(json::w::Document& document)
	{

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
					int id = 0;
					std::string name;
					LOG_CHECK_RET_FALSE(jsonObject->Get("id", id))
					LOG_CHECK_RET_FALSE(jsonObject->Get("name", name))
					std::unique_ptr<Server> server1= std::make_unique<Server>(id, name);
					{
						if(!server1->Decode(*jsonObject))
						{
							return false;
						}
						this->AddActor(std::move(server1));
					}
				}
			}
		}
		return true;
	}

	bool ActorComponent::DelActor(long long id)
	{
		if(id == this->mApp->Equal(id))
		{
			return false;
		}
		auto iter = this->mActors.find(id);
		if(iter == this->mActors.end())
		{
			return false;
		}
		this->mActors.erase(iter);
		return true;
	}

	bool ActorComponent::AddGroup(const std::string& name, long long id)
	{
		auto iter = this->mGroups.find(name);
		if(iter == this->mGroups.end())
		{
			actor::Group group;
			group.AddItem(id);
			this->mGroups.emplace(name, group);
			return true;
		}
		return iter->second.AddItem(id);
	}

	bool ActorComponent::GetListen(int id, const std::string& net, std::string& address)
	{
		Actor * actor = this->GetActor(id);
		if(actor == nullptr)
		{
			return false;
		}
		return static_cast<Server*>(actor)->GetListen(net, address);
	}

	actor::Group* ActorComponent::GetGroup(const std::string& name)
	{
		auto iter = this->mGroups.find(name);
		if(iter == this->mGroups.end())
		{
			return nullptr;
		}
		return &iter->second;
	}

	Actor* ActorComponent::GetActor(long long id)
	{
		if(this->mApp->Equal(id))
		{
			return this->mApp;
		}
		auto iter = this->mActors.find(id);
		return iter != this->mActors.end() ? iter->second.get() : nullptr;
	}

	bool ActorComponent::AddActor(std::unique_ptr<Actor> actor, bool group)
	{
		long long id = actor->GetId();
		auto iter = this->mActors.find(id);
		if(iter != this->mActors.end())
		{
			return false;
		}
		if(!actor->LateAwake())
		{
			return false;
		}
		if(group)
		{
			this->AddGroup(actor->Name(), id);
		}
		this->mActors.emplace(id, std::move(actor));
		return true;
	}
}