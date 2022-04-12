//
// Created by mac on 2022/4/12.
//

#include"MethodRegister.h"

namespace Sentry
{
	void SubServiceRegister::GetMethods(std::vector<std::string>& methods)
	{

	}

	std::shared_ptr<SubMethod> SubServiceRegister::GetMethod(const std::string& func)
	{
		auto iter = this->mSubMethodMap.find(func);
		if (iter != this->mSubMethodMap.end())
		{
			return iter->second;
		}
		return nullptr;
	}

}