#include"GameObjectComponent.h"
#include"Core/App.h"
#include"Object/GameObject.h"
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

    bool GameObjectComponent::Add(GameObject * gameObject)
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

    GameObject * GameObjectComponent::Find(const std::string & address)
    {
        auto iter = this->mAddressGameObjects.find(address);
        return iter != this->mAddressGameObjects.end() ? iter->second : nullptr;
    }

    bool GameObjectComponent::Del(GameObject * gameObject)
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
		GameObject * gameObject = iter->second;
		if (gameObject != nullptr)
		{
			gameObject->OnDestory();
			this->mGameObjects.erase(iter);
			delete gameObject;
		}
		return true;
	}

	GameObject * GameObjectComponent::Find(long long id)
	{
		auto iter = this->mGameObjects.find(id);
		if (iter == this->mGameObjects.end())
		{
			return nullptr;
		}
		
		GameObject * gameObject = iter->second;
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

	void GameObjectComponent::GetGameObjects(std::vector<GameObject *> & gameObjects)
	{
		auto iter = this->mGameObjects.begin();
		for (; iter != this->mGameObjects.end(); iter++)
		{
			GameObject * gameObject = iter->second;
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
        GameObject * gameObject = this->Find(objectId);
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