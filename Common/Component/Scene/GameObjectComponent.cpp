#include"GameObjectComponent.h"
#include"Object/App.h"
#include"Object/Entity.h"
#include"Coroutine/TaskComponent.h"

namespace Sentry
{
    bool GameObjectComponent::Awake()
    {
        this->mCorComponent = nullptr;
        return true;
    }

    bool GameObjectComponent::LateAwake()
    {
        this->mCorComponent = App::Get().GetTaskComponent();
        return true;
    }

    bool GameObjectComponent::Add(Entity * gameObject)
	{
		if (gameObject == nullptr)
		{
			return false;
		}
		long long id = gameObject->GetId();
		auto iter = this->mGameObjects.find(id);
		if (iter != this->mGameObjects.end())
		{
			return false;
		}
		std::vector<Component *> components;
		gameObject->GetComponents(components);
		for (Component * component : components)
		{
			if (!component->LateAwake())
            {
                LOG_ERROR("Init", component->GetTypeName(), "failure");
                return false;
            }
		}
		this->mGameObjects.emplace(id, gameObject);
        this->mCorComponent->Start(&GameObjectComponent::StartComponents, this, id);
		return true;
	}

    Entity * GameObjectComponent::Find(const std::string & address)
    {
        auto iter = this->mAddressGameObjects.find(address);
        return iter != this->mAddressGameObjects.end() ? iter->second : nullptr;
    }

    bool GameObjectComponent::Del(Entity * gameObject)
	{
		long long id = gameObject->GetId();
		return this->Del(id);
	}
	bool GameObjectComponent::Del(long long id)
	{
		auto iter = this->mGameObjects.find(id);
		if (iter == this->mGameObjects.end())
		{
			return false;
		}
		Entity * gameObject = iter->second;
		if (gameObject != nullptr)
		{
			gameObject->OnDestory();
			this->mGameObjects.erase(iter);
			delete gameObject;
		}
		return true;
	}

	Entity * GameObjectComponent::Find(long long id)
	{
		auto iter = this->mGameObjects.find(id);
		if (iter == this->mGameObjects.end())
		{
			return nullptr;
		}
		
		Entity * gameObject = iter->second;
		if (gameObject == nullptr)
		{
			return nullptr;
		}
		if (!gameObject->IsActive())
		{
			gameObject->OnDestory();
			this->mGameObjects.erase(iter);
			delete gameObject;
			return nullptr;
		}
		return gameObject;
	}

	void GameObjectComponent::GetGameObjects(std::vector<Entity *> & gameObjects)
	{
		auto iter = this->mGameObjects.begin();
		for (; iter != this->mGameObjects.end(); iter++)
		{
			Entity * gameObject = iter->second;
			if (!gameObject->IsActive())
			{
				gameObject->OnDestory();
				this->mGameObjects.erase(iter);
				delete gameObject;
				continue;
			}
			gameObjects.push_back(gameObject);
		}
	}

	void GameObjectComponent::StartComponents(long long objectId)
	{
        Entity * gameObject = this->Find(objectId);
        if(gameObject != nullptr)
        {
            std::vector<Component *> components;
            gameObject->GetComponents(components);
            for (Component *component: components)
            {
                auto startComponent = dynamic_cast<IStart *>(component);
                if (startComponent != nullptr);
                {
                    startComponent->OnStart();
                }
            }
        }
	}
}