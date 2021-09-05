#include "ComponentHelper.h"
namespace Sentry
{

	std::unordered_map<size_t, Type *> ComponentHelper::mTypeInfoMap1;
	std::unordered_map<std::string, Type *> ComponentHelper::mTypeInfoMap;
	std::unordered_map<size_t, std::queue<Component *>> ComponentHelper::mComponentPool;

	Sentry::Type * ComponentHelper::GetType(const std::string name)
	{
		auto iter = mTypeInfoMap.find(name);
		return iter != mTypeInfoMap.end() ? iter->second : nullptr;
	}

	bool ComponentHelper::DestoryComponent(Component * component)
	{
		if (component == nullptr)
		{
			return true;
		}			
		Type * type = component->GetType();
		auto iter = mComponentPool.find(type->Hash);
		if (iter != mComponentPool.end())
		{
			if (iter->second.size() < 100)
			{
				component->OnDestory();
				component->gameObjectID = 0;
				component->gameObject = nullptr;
				iter->second.push(component);
				return false;
			}
		}
		delete component;
		return true;
	}

	Sentry::Component * ComponentHelper::CreateComponent(const std::string & name, bool fromPool /*= true*/)
	{
		auto iter = mTypeInfoMap.find(name);
		if (iter == mTypeInfoMap.end())
		{
			return nullptr;
		}
		Type * type = iter->second;

		auto iter1 = mComponentPool.find(type->Hash);
		if (iter1 != mComponentPool.end())
		{
			std::queue<Component *> & components = iter1->second;
			if (!components.empty())
			{
				Component * component = components.front();
				components.pop();
				return component;
			}
		}

		Component * component = type->New();
		if (component != nullptr)
		{
			component->mType = type;
			component->Init(type->Name);
		}
		return component;
	}

}