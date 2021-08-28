#include "ComponentHelper.h"
namespace Sentry
{

	std::unordered_map<size_t, Type *> ComponentHelper::mTypeInfoMap1;
	std::unordered_map<std::string, Type *> ComponentHelper::mTypeInfoMap;

	Sentry::Type * ComponentHelper::Get(const std::string name)
	{
		auto iter = mTypeInfoMap.find(name);
		return iter != mTypeInfoMap.end() ? iter->second : nullptr;
	}

}