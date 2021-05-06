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
}


