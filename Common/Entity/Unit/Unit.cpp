#include"Unit.h"
#include"App/App.h"
#include"Define/CommonLogDef.h"
#include"Component/Component.h"
namespace Sentry
{
	Unit::Unit(long long id) : mUnitId(id)
	{
	}

	bool Unit::AddComponent(const std::string& name)
	{
		auto iter = this->mComponentMap.find(name);
		if (iter != this->mComponentMap.end())
		{
			return false;
		}
		return this->AddComponent(name, ComponentFactory::CreateComponent(name));
	}

	bool Unit::AddComponent(const std::string& name, Component* component)
	{
		if (component == nullptr)
		{
			return false;
		}
		auto iter = this->mComponentMap.find(name);
		if (iter != this->mComponentMap.end())
		{
			LOG_ERROR("add " << name << " failure");
			return false;
		}
		component->mName = name;
		component->mEntityId = mUnitId;
		component->mEntity = this->shared_from_this();

		component->Awake();
		this->OnAddComponent(component);
		this->mSortComponents.emplace_back(name);
		this->mComponentMap.emplace(name, component);
		return true;
	}

	size_t Unit::GetComponents(std::vector<Component*>& components) const
	{
		auto iter = this->mComponentMap.begin();
		for(;iter != this->mComponentMap.end(); iter++)
		{
			components.emplace_back(iter->second);
		}
        return components.size();
	}

	size_t Unit::GetComponents(std::vector<std::string>& components) const
	{
		components.clear();
		for (const std::string& name : this->mSortComponents)
		{
			Component* component = this->GetComponent<Component>(name);
			if (component != nullptr)
			{
				components.emplace_back(component->GetName());
			}
		}
        return components.size();
	}

	void Unit::OnDestory()
	{
		auto iter = this->mComponentMap.begin();
		for (; iter != this->mComponentMap.end(); iter++)
		{
			Component* component = iter->second;
			if (component != nullptr)
			{
				component->OnDestory();
				component->SetActive(false);
				ComponentFactory::DestoryComponent(component);
			}
		}
		this->mComponentMap.clear();
	}

	Component* Unit::GetComponentByName(const std::string& name)
	{
		auto iter = this->mComponentMap.find(name);
		return iter != this->mComponentMap.end() ? iter->second : nullptr;
	}

	bool Unit::RemoveComponent(const std::string& name)
	{
		auto iter = this->mComponentMap.find(name);
		if (iter != this->mComponentMap.end())
		{
			Component* component = iter->second;
			this->mComponentMap.erase(iter);
			if (component != nullptr)
			{
				component->OnDestory();
				component->SetActive(false);
				this->OnDelComponent(component);
				ComponentFactory::DestoryComponent(component);
				return true;
			}
		}
		return false;
	}
}// namespace Sentry
