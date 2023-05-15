#include"PlayerMgrComponent.h"
#include"Entity/Unit/App.h"
#include"Entity/Unit/Player.h"
namespace Tendo
{
	bool PlayerMgrComponent::LateAwake()
	{
		this->mCorComponent = this->mApp->GetCoroutine();
		return true;
	}

	bool PlayerMgrComponent::Add(std::unique_ptr<Player> player)
	{
		LOG_CHECK_RET_FALSE(player);
		long long id = player->GetUnitId();
		auto iter = this->mPlayers.find(id);
		if(iter != this->mPlayers.end())
        {
            CONSOLE_LOG_ERROR("add unit error id : " << id);
            return false;
        }

		std::vector<Component *> components;
		player->GetComponents(components);
		for (Component * component : components)
		{
			if (!component->LateAwake())
			{
				LOG_ERROR("Init" << component->GetName() << "failure");
				return false;
			}
		}
		this->mPlayers.emplace(id, std::move(player));
		return true;
	}

	bool PlayerMgrComponent::DelPlayer(long long id)
	{
		auto iter = this->mPlayers.find(id);
		if (iter == this->mPlayers.end())
		{
			return false;
		}
		this->mPlayers.erase(iter);
		return true;
	}

	Player * PlayerMgrComponent::GetPlayer(long long id)
	{
		auto iter = this->mPlayers.find(id);
		if (iter == this->mPlayers.end())
		{
			return nullptr;
		}
        return iter->second.get();
	}

	void PlayerMgrComponent::GetPlayers(std::vector<Player*>& gameObjects)
	{
		auto iter = this->mPlayers.begin();
		for (; iter != this->mPlayers.end(); iter++)
		{
			gameObjects.push_back(iter->second.get());
		}
	}

	void PlayerMgrComponent::StartComponents(long long objectId)
	{
		std::vector<IStart*> components;
		Player * unit = this->GetPlayer(objectId);
		if (unit != nullptr && unit->GetComponents(components) > 0)
		{
			for (IStart* component : components)
			{
				component->Start();
			}
		}
	}
}