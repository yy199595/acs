#include "ManagerFactory.h"

namespace SoEasy
{
	Manager * ManagerFactory::Create(const std::string name)
	{
		auto iter = this->mCreateActions.find(name);
		if (iter == this->mCreateActions.end())
		{
			return nullptr;
		}
		return iter->second();
	}
	bool ManagerFactory::GetTypeName(const size_t hash, std::string & name)
	{
		auto iter = this->mTypeInfoMap.find(hash);
		if (iter != this->mTypeInfoMap.end())
		{
			name = iter->second;
			return true;
		}
		return false;
	}
}


