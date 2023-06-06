#include"Entity.h"
#include"Entity/Actor/App.h"
#include"Entity/Component/Component.h"
namespace Tendo
{
	Entity::Entity(long long id) : mEntityId(id)
	{
	}

	bool Entity::HasComponent(const std::string& name)
	{
		auto iter = this->mComponentMap.find(name);
		return iter != this->mComponentMap.end();
	}

	bool Entity::AddComponent(const std::string& name)
	{
		std::unique_ptr<Component> component = ComponentFactory::CreateComponent(name);
		if(component == nullptr)
		{
			return false;
		}
		return this->AddComponent(name, std::move(component));
	}

	bool Entity::AddComponent(const std::string& name, std::unique_ptr<Component> component)
	{
		if(this->HasComponent(name))
		{
			return false;
		}
		component->mName = name;
        component->mUnit = this;
        component->mEntityId = mEntityId;

		if(!component->Awake())
        {
            return false;
        }
        this->mSortComponents.emplace_back(name);
        this->OnAddComponent(component.get());
		this->mComponentMap.emplace(name, std::move(component));
		return true;
	}


	size_t Entity::GetComponents(std::vector<Component*>& components) const
	{
        for(const std::string & name : this->mSortComponents)
        {
            Component * component = this->GetComponentByName(name);
            if(component != nullptr)
            {
                components.emplace_back(component);
            }
        }
        return components.size();
	}

	size_t Entity::GetComponents(std::vector<std::string>& components) const
	{
		components.clear();
		components.insert(components.end(),
			this->mSortComponents.begin(), this->mSortComponents.end());
        return components.size();
	}

	Component* Entity::GetComponentByName(const std::string& name) const
	{
		auto iter = this->mComponentMap.find(name);
		return iter != this->mComponentMap.end() ? iter->second.get() : nullptr;
	}

	bool Entity::RemoveComponent(const std::string& name)
	{
		auto iter = this->mComponentMap.find(name);
		if (iter != this->mComponentMap.end())
		{
			Component * component = iter->second.get();
            if(!this->OnDelComponent(component))
            {
                return false;
            }
			this->mComponentMap.erase(iter);
			return true;
		}
		return false;
	}
}// namespace Sentry
