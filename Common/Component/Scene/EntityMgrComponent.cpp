#include"EntityMgrComponent.h"
#include"App/App.h"
#include"Entity/Entity.h"
namespace Sentry
{
    bool EntityMgrComponent::Awake()
    {
        this->mCorComponent = nullptr;
        return true;
    }

    bool EntityMgrComponent::LateAwake()
    {
        this->mCorComponent = App::Get()->GetTaskComponent();
        return true;
    }

    bool EntityMgrComponent::Add(std::shared_ptr<Entity> gameObject)
	{
        LOG_CHECK_RET_FALSE(gameObject);
		long long id = gameObject->GetId();
		auto iter = this->mGameObjects.find(id);
        LOG_CHECK_RET_FALSE(iter == this->mGameObjects.end());
		std::vector<Component *> components;
		gameObject->GetComponents(components);
		for (Component * component : components)
		{
			if (!component->LateAwake())
            {
                LOG_ERROR("Init", component->GetName(), "failure");
                return false;
            }
		}
		this->mGameObjects.emplace(id, gameObject);
        this->mCorComponent->Start(&EntityMgrComponent::StartComponents, this, id);
		return true;
	}

    bool EntityMgrComponent::Del(std::shared_ptr<Entity> gameObject)
	{
		long long id = gameObject->GetId();
		return this->Del(id);
	}
	bool EntityMgrComponent::Del(long long id)
	{
		auto iter = this->mGameObjects.find(id);
		if (iter == this->mGameObjects.end())
		{
			return false;
		}
		std::shared_ptr<Entity> gameObject = iter->second;
		if (gameObject != nullptr)
		{
			gameObject->OnDestory();
			this->mGameObjects.erase(iter);
		}
		return true;
	}

	std::shared_ptr<Entity> EntityMgrComponent::Find(long long id)
	{
		auto iter = this->mGameObjects.find(id);
		if (iter == this->mGameObjects.end())
		{
			return nullptr;
		}
		
		auto gameObject = iter->second;
		if (!gameObject->IsActive())
		{
			gameObject->OnDestory();
			this->mGameObjects.erase(iter);
			return nullptr;
		}
		return gameObject;
	}

	void EntityMgrComponent::GetGameObjects(std::vector<std::shared_ptr<Entity>> & gameObjects)
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

	void EntityMgrComponent::StartComponents(long long objectId)
    {
        auto gameObject = this->Find(objectId);
        LOG_CHECK_RET(gameObject);
        std::vector<Component *> components;
        gameObject->GetComponents(components);
        for (Component *component: components)
        {
            IStart *start = dynamic_cast<IStart *>(component);
            ILoadData *loadData = dynamic_cast<ILoadData *>(component);
            if (start != nullptr)
            {
                start->OnStart();
            }
            if (loadData != nullptr)
            {
                loadData->OnLoadData();
            }
        }
    }
}