//
// Created by yjz on 2022/1/22.
//
#include"SubService.h"

namespace Sentry
{
	void SubService::GetSubMethods(std::vector<std::string>& methods)
	{
		auto iter = this->mSubMethodMap.begin();
		for (; iter != this->mSubMethodMap.end(); iter++)
		{
			methods.emplace_back(iter->first);
		}
	}

	bool SubService::Publish(const std::string& func, const std::string& message)
	{
		auto iter = this->mSubMethodMap.find(func);
		if (iter != this->mSubMethodMap.end())
		{
			iter->second->OnPublish(message);
			return true;
		}
		return false;
	}
}
