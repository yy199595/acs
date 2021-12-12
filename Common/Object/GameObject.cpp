#include "GameObject.h"
#include"Core/App.h"
#include <Define/CommonLogDef.h>
#include <Component/Component.h>
namespace GameKeeper
{
    GameObject::GameObject(long long id)
        : mGameObjectId(id), mSocketId(0)
    {
    }

    GameObject::GameObject(long long id, long long socketId)
        : mGameObjectId(id), mSocketId(socketId)
    {

    }


	bool GameObject::AddComponent(const std::string & name)
	{		
		auto iter = this->mComponentMap.find(name);
		if (iter != this->mComponentMap.end())
		{
			return false;
		}
		return this->AddComponent(name, ComponentHelper::CreateComponent(name));
	}

	bool GameObject::AddComponent(const std::string & name, Component * component)
	{
		if (component == nullptr)
		{
			return false;
		}
		auto iter = this->mComponentMap.find(name);
		if (iter != this->mComponentMap.end())
		{
			LOG_ERROR("add " << name << "failure");
			return false;
		}
		component->gameObject = this;
		component->gameObjectID = mGameObjectId;
        if(!component->Awake())
        {
            return false;
        }
		this->mComponentMap.emplace(name, component);
		return true;
	}

	void GameObject::GetComponents(std::vector<Component*>& components, bool sort) const
	{
		components.clear();
		auto iter = this->mComponentMap.begin();
		for (; iter != this->mComponentMap.end(); iter++)
		{
			Component *component = iter->second;
			components.push_back(component);
		}
		if (sort)
		{
			std::sort(components.begin(), components.end(),
				[](Component * c1, Component * c2)
			{
				return c1->GetPriority() < c2->GetPriority();
			});
		}
	}

	void GameObject::OnDestory()
    {
		auto iter = this->mComponentMap.begin();
		for (; iter != this->mComponentMap.end(); iter++)
		{
			Component * component = iter->second;
			if (component != nullptr)
			{
				component->OnDestory();
				component->SetActive(false);
				ComponentHelper::DestoryComponent(component);
			}			
		}
		this->mComponentMap.clear();
    }

	Component * GameObject::GetComponentByName(const std::string & name)
	{
		auto iter = this->mComponentMap.find(name);
		return iter != this->mComponentMap.end() ? iter->second : nullptr;
	}

	bool GameObject::RemoveComponent(const std::string &name)
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
				ComponentHelper::DestoryComponent(component);
				return true;
			}
        }
        return false;
    }
}// namespace GameKeeper
