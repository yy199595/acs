#include "GameObject.h"
#include <Define/CommonDef.h>
#include <Component/Component.h>
namespace Sentry
{
    GameObject::GameObject(const long long id)
        : mGameObjectId(id)
    {
    }

    GameObject::GameObject(const long long id, const std::string &address)
        : mGameObjectId(id)
    {
        mSessionAddress = address;
    }


	bool GameObject::AddComponent(const std::string & name)
	{		
		auto iter = this->mComponentMap.find(name);
		if (iter != this->mComponentMap.end())
		{
			return false;
		}

		Component * component = ComponentHelper::CreateComponent(name);
		return this->AddComponent(name, component);
	}

	bool GameObject::AddComponent(const std::string name, Component * component)
	{
		if (component == nullptr)
		{
			return false;
		}
		auto iter = this->mComponentMap.find(name);
		if (iter != this->mComponentMap.end())
		{
			SayNoDebugError("add " << name << "failure");
			return false;
		}
		component->gameObject = this;
		component->gameObjectID = mGameObjectId;
		this->mComponentMap.emplace(name, component);
		return true;
	}

	void GameObject::GetComponents(std::vector<Component*>& components) const
	{
		components.clear();
		auto iter = this->mComponentMap.begin();
		for (; iter != this->mComponentMap.end(); iter++)
		{
			Component *component = iter->second;
			components.push_back(component);
		}
	}

	void GameObject::OnDestory()
    {
		auto iter = this->mComponentMap.begin();
		for (; iter != this->mComponentMap.end(); iter++)
		{
			Component * component = iter->second;
			component->OnDestory();
		}
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
            component->SetActive(false);
			this->mComponentMap.erase(iter);
			ComponentHelper::DestoryComponent(component);
            return true;
        }
        return false;
    }
}// namespace Sentry
