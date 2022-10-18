#include "ComponentFactory.h"
namespace Sentry
{

	std::unordered_map<size_t, Type*> ComponentFactory::mTypeInfoMap1;
	std::unordered_map<std::string, Type*> ComponentFactory::mTypeInfoMap;

	Sentry::Type* ComponentFactory::GetType(const std::string& name)
	{
		auto iter = mTypeInfoMap.find(name);
		return iter != mTypeInfoMap.end() ? iter->second : nullptr;
	}

	bool ComponentFactory::DestoryComponent(Component* component)
	{
		if (component == nullptr)
		{
			return false;
		}
		component->OnDestory();
		component->mEntityId = 0;
		component->mUnit = nullptr;
		component->SetActive(false);
		delete component;
		return true;
	}

	std::unique_ptr<Component> ComponentFactory::CreateComponent(const std::string& name, bool fromPool /*= true*/)
	{
		auto iter = mTypeInfoMap.find(name);
		if (iter == mTypeInfoMap.end())
		{
			return nullptr;
		}
		Type* type = iter->second;
		std::unique_ptr<Component> component = type->New();
		if (component != nullptr)
		{
			component->Init();
			component->mType = type;
		}
		return std::move(component);
	}
}