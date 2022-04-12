//
// Created by mac on 2022/4/12.
//

#include"MethodRegister.h"

namespace Sentry
{
	void SubServiceRegister::GetMethods(std::vector<std::string>& methods)
	{
		auto iter = this->mSubMethodMap.begin();
		for(; iter != this->mSubMethodMap.end(); iter++)
		{
			methods.emplace_back(iter->first);
		}
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

	std::shared_ptr<HttpServiceMethod> HttpServiceRegister::GetMethod(const string& name)
	{
		auto iter = this->mHttpMethodMap.find(name);
		return iter != this->mHttpMethodMap.end() ? iter->second : nullptr;
	}
}