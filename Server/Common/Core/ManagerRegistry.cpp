#include "ManagerRegistry.h"

namespace SoEasy
{
	std::unordered_map<size_t, std::string> ManagerRegistry::mTypeInfoMap;
std::unordered_map<std::string, std::function<Manager *()>> ManagerRegistry::mCreateActions;
	Manager * ManagerRegistry::Create(const std::string name)
	{
		auto iter = mCreateActions.find(name);
		if (iter == mCreateActions.end())
		{
			return nullptr;
		}
		return iter->second();
	}
	bool ManagerRegistry::GetTypeName(const size_t hash, std::string & name)
	{
		auto iter = mTypeInfoMap.find(hash);
		if (iter != mTypeInfoMap.end())
		{
			name = iter->second;
			return true;
		}
		return false;
	}
}


