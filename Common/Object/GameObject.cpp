#include "GameObject.h"
#include <Core/App.h>
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
		Type * type = ComponentHelper::Get(name);
		if (type == nullptr)
		{
			SayNoDebugError("create " << name << " failure");
			return false;
		}

		auto iter = this->mComponentMap.find(name);
		if (iter != this->mComponentMap.end())
		{
			return false;
		}

		Component * component = type->Create();
		return this->AddComponent(name, component);		
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
			SayNoDebugError("add " << name << "failure");
			return false;
		}
		if (component->Init(name))
		{
			component->gameObject = this;
			component->gameObjectID = mGameObjectId;
			this->mComponentMap.emplace(name, component);
			return true;
		}
		return false;
	}

	void GameObject::GetComponents(std::vector<Component*>& components)
	{
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

    bool GameObject::RemoveComponent(const std::string &name)
    {
        auto iter = this->mComponentMap.find(name);
        if (iter != this->mComponentMap.end())
        {
            Component *component = iter->second;
            component->SetActive(false);
			this->mComponentMap.erase(iter);
            return true;
        }
        return false;
    }
}// namespace Sentry
