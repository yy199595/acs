//
// Created by yjz on 2023/5/17.
//

#include"ActorMgrComponent.h"
#include"Entity/Actor/App.h"
#include"Server/Config/ServerConfig.h"
namespace Tendo
{
	bool ActorMgrComponent::AddActor(Actor* actor)
	{
		long long id = actor->GetUnitId();
		auto iter = this->mActors.find(id);
		if(iter != this->mActors.end())
		{
			return false;
		}
		const std::string & name = actor->GetName();
		const std::string & addr = actor->GetActorAddr();
		{
			this->mActors.emplace(id, actor);
			this->mAddrActors.emplace(addr, actor);
		}
		if(!name.empty())
		{
			auto iter1 = this->mActorNames.find(name);
			if(iter1 == this->mActorNames.end())
			{
				std::vector<long long> ret;
				this->mActorNames.emplace(name, ret);
			}
			this->mActorNames[name].emplace_back(id);
		}
		return actor->LateAwake();
	}

	Actor* ActorMgrComponent::GetOrCreateActor(long long id, const std::string& addr)
	{
		Actor * actor = this->GetActor(id);
		if(actor != nullptr)
		{
			return actor;
		}
		actor = new Actor(id, addr);
		if(!this->AddActor(actor))
		{
			delete actor;
			return nullptr;
		}
		return actor;
	}

	Actor* ActorMgrComponent::RandomActor(const std::string& name)
	{
		auto iter = this->mActorNames.find(name);
		if(iter == this->mActorNames.end())
		{
			return nullptr;
		}
		int index = rand() % iter->second.size();
		return this->GetActor(iter->second[index]);
	}

	Actor* ActorMgrComponent::GetActor(long long id)
	{
		auto iter = this->mActors.find(id);
		return iter != this->mActors.end() ? iter->second : nullptr;
	}

	Actor* ActorMgrComponent::GetActor(const std::string& addr)
	{
		auto iter = this->mAddrActors.find(addr);
		return iter != this->mAddrActors.end() ? iter->second : nullptr;
	}

	bool ActorMgrComponent::DelActor(long long id)
	{
		return true;
	}

	bool ActorMgrComponent::DelActor(const std::string& addr)
	{
		return true;
	}
}