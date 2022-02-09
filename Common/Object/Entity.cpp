#include "Entity.h"
#include"App.h"
#include <Define/CommonLogDef.h>
#include <Component/Component.h>
namespace Sentry
{
    Entity::Entity(long long id)
        : mGameObjectId(id), mSocketId(0)
    {
    }

    Entity::Entity(long long id, long long socketId)
        : mGameObjectId(id), mSocketId(socketId)
    {

    }


	bool Entity::AddComponent(const std::string & name)
	{		
		auto iter = this->mComponentMap.find(name);
		if (iter != this->mComponentMap.end())
		{
			return false;
		}
		return this->AddComponent(name, ComponentFactory::CreateComponent(name));
	}

	bool Entity::AddComponent(const std::string & name, Component * component)
	{
		if (component == nullptr)
		{
			return false;
		}
		auto iter = this->mComponentMap.find(name);
		if (iter != this->mComponentMap.end())
		{
			LOG_ERROR("add {0} failure", name);
			return false;
		}
		component->mEntity = this;
		component->mEntityId = mGameObjectId;
        if(!component->Awake())
        {
            return false;
        }
        this->OnAddComponent(component);
        this->mSortComponents.emplace_back(name);
		this->mComponentMap.emplace(name, component);
		return true;
	}

	void Entity::GetComponents(std::vector<Component*>& components) const
	{
		components.clear();
        for(const std::string & name : this->mSortComponents)
        {
           Component * component = this->GetComponent<Component>(name);
           if(component != nullptr)
           {
               components.emplace_back(component);
           }
        }
	}

	void Entity::OnDestory()
    {
		auto iter = this->mComponentMap.begin();
		for (; iter != this->mComponentMap.end(); iter++)
		{
			Component * component = iter->second;
			if (component != nullptr)
			{
				component->OnDestory();
				component->SetActive(false);
				ComponentFactory::DestoryComponent(component);
			}			
		}
		this->mComponentMap.clear();
    }

	Component * Entity::GetComponentByName(const std::string & name)
	{
		auto iter = this->mComponentMap.find(name);
		return iter != this->mComponentMap.end() ? iter->second : nullptr;
	}

	bool Entity::RemoveComponent(const std::string &name)
    {
        auto iter = this->mComponentMap.find(name);
        if (iter != this->mComponentMap.end())
        {
            Component *component = iter->second;
			this->mComponentMap.erase(iter);
			if (component != nullptr)
			{
				component->OnDestory();
				component->SetActive(false);
				ComponentFactory::DestoryComponent(component);
				return true;
			}
        }
        return false;
    }
}// namespace Sentry
