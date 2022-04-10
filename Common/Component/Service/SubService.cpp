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

	bool SubService::Publish(const std::string& func, const Json::Reader & jsonReader)
	{
		auto iter = this->mSubMethodMap.find(func);
		if (iter != this->mSubMethodMap.end())
		{
			iter->second->OnPublish(jsonReader);
			return true;
		}
		return false;
	}
}
