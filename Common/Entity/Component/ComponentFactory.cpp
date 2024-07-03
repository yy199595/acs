#include "ComponentFactory.h"
namespace joke
{
	std::string ComponentFactory::mEmpty;
	std::unordered_map<size_t, Type*> ComponentFactory::mTypeInfoMap1;
	std::unordered_map<std::string, Type*> ComponentFactory::mTypeInfoMap;

	joke::Type* ComponentFactory::GetType(const std::string& name)
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
		Type* type = iter->second;
		return std::move(type->New());
	}
}