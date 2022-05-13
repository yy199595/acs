//
// Created by mac on 2022/4/12.
//

#include"MethodRegister.h"

namespace Sentry
{
	std::shared_ptr<HttpServiceMethod> HttpServiceRegister::GetMethod(const string& name)
	{
		auto iter = this->mHttpMethodMap.find(name);
		return iter != this->mHttpMethodMap.end() ? iter->second : nullptr;
	}
}