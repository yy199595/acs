#include"UnitMgrComponent.h"
#include"App/App.h"
#include"Unit/Unit.h"
namespace Sentry
{
	bool UnitMgrComponent::LateAwake()
	{
		this->mCorComponent = App::Get()->GetTaskComponent();
		return true;
	}

	bool UnitMgrComponent::Add(std::shared_ptr<Unit> gameObject)
	{
		LOG_CHECK_RET_FALSE(gameObject);
		long long id = gameObject->GetUnitId();
		auto iter = this->mGameObjects.find(id);
		if(iter != this->mGameObjects.end())
        {
            CONSOLE_LOG_ERROR("add unit error id : " << id);
            return false;
        }

		std::vector<Component *> components;
		gameObject->GetComponents(components);
		for (Component * component : components)
		{
			if (!component->LateAwake())
			{
				LOG_ERROR("Init" << component->GetName() << "failure");
				return false;
			}
		}
		this->mGameObjects.emplace(id, gameObject);
		this->mCorComponent->Start(&UnitMgrComponent::StartComponents, this, id);
		return true;
	}

	bool UnitMgrComponent::Del(std::shared_ptr<Unit> gameObject)
	{
		long long id = gameObject->GetUnitId();
		return this->Del(id);
	}
	bool UnitMgrComponent::Del(long long id)
	{
		auto iter = this->mGameObjects.find(id);
		if (iter == this->mGameObjects.end())
		{
			return false;
		}
		std::shared_ptr<Unit> gameObject = iter->second;
		if (gameObject != nullptr)
		{
			gameObject->OnDestory();
			this->mGameObjects.erase(iter);
		}
		return true;
	}

	std::shared_ptr<Unit> UnitMgrComponent::Find(long long id)
	{
		auto iter = this->mGameObjects.find(id);
		if (iter == this->mGameObjects.end())
		{
			return nullptr;
		}
        return iter->second;
	}

	void UnitMgrComponent::GetGameObjects(std::vector<std::shared_ptr<Unit>>& gameObjects)
	{
		auto iter = this->mGameObjects.begin();
		for (; iter != this->mGameObjects.end(); iter++)
		{
			auto gameObject = iter->second;
			if (!gameObject->IsActive())
			{
				gameObject->OnDestory();
				this->mGameObjects.erase(iter);
				continue;
			}
			gameObjects.push_back(gameObject);
		}
	}

	void UnitMgrComponent::StartComponents(long long objectId)
	{
		auto gameObject = this->Find(objectId);
		LOG_CHECK_RET(gameObject);
		std::vector<std::string> components;
		gameObject->GetComponents(components);
		for (const std::string & name : components)
		{
			Component * component = gameObject->GetComponent<Component>(name);
			if(component != nullptr)
			{
				IStart * start = component->Cast<IStart>();
				if(start != nullptr)
				{
                    start->Start();
				}
			}
		}
	}
}