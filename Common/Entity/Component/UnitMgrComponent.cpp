#include"UnitMgrComponent.h"
#include"Entity/Unit/App.h"
namespace Tendo
{
	bool UnitMgrComponent::LateAwake()
	{
		this->mCorComponent = this->mApp->GetCoroutine();
		return true;
	}

	bool UnitMgrComponent::Add(std::unique_ptr<Unit> unit)
	{
		LOG_CHECK_RET_FALSE(unit);
		long long id = unit->GetUnitId();
		auto iter = this->mGameObjects.find(id);
		if(iter != this->mGameObjects.end())
        {
            CONSOLE_LOG_ERROR("add unit error id : " << id);
            return false;
        }

		std::vector<Component *> components;
		unit->GetComponents(components);
		for (Component * component : components)
		{
			if (!component->LateAwake())
			{
				LOG_ERROR("Init" << component->GetName() << "failure");
				return false;
			}
		}
		this->mGameObjects.emplace(id, std::move(unit));
		this->mCorComponent->Start(&UnitMgrComponent::StartComponents, this, id);
		return true;
	}

	bool UnitMgrComponent::Del(long long id)
	{
		auto iter = this->mGameObjects.find(id);
		if (iter == this->mGameObjects.end())
		{
			return false;
		}
		this->mGameObjects.erase(iter);
		return true;
	}

	Unit * UnitMgrComponent::Find(long long id)
	{
		auto iter = this->mGameObjects.find(id);
		if (iter == this->mGameObjects.end())
		{
			return nullptr;
		}
        return iter->second.get();
	}

	void UnitMgrComponent::GetUnits(std::vector<Unit*>& gameObjects)
	{
		auto iter = this->mGameObjects.begin();
		for (; iter != this->mGameObjects.end(); iter++)
		{
			gameObjects.push_back(iter->second.get());
		}
	}

	void UnitMgrComponent::StartComponents(long long objectId)
	{
		std::vector<IStart*> components;
		Unit* unit = this->Find(objectId);
		if (unit != nullptr && unit->GetComponents(components) > 0)
		{
			for (IStart* component : components)
			{
				component->Start();
			}
		}
	}
}