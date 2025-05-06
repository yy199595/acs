#include "ComponentFactory.h"
namespace acs
{
	std::string ComponentFactory::mEmpty;
	std::unordered_map<std::string, Type*> ComponentFactory::mTypeInfoMap;
	std::unordered_map<size_t, std::unique_ptr<Type>> ComponentFactory::mTypeInfoMap1;

	acs::Type* ComponentFactory::GetType(const std::string& name)
	{
		auto iter = mTypeInfoMap.find(name);
		return iter != mTypeInfoMap.end() ? iter->second : nullptr;
	}

	std::unique_ptr<Component> ComponentFactory::CreateComponent(const std::string& name)
	{
		auto iter = mTypeInfoMap.find(name);
		if (iter == mTypeInfoMap.end())
		{
			return nullptr;
		}
		return std::move(iter->second->New());
	}
}