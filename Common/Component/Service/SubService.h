//
// Created by yjz on 2022/1/22.
//
#include"Component/Component.h"
#include"Method/SubMethod.h"
#ifndef SENTRY_REDISSUBSERVICE_H
#define SENTRY_REDISSUBSERVICE_H

namespace Sentry
{
	class SubService : public Component
	{
	 public:
		SubService() = default;
		virtual ~SubService() = default;
	 public:
		void GetSubMethods(std::vector<std::string>& methods);
		bool Publish(const std::string& func, const Json::Reader & jsonReader);
	 protected:
		template<typename T>
		bool Bind(std::string name, JsonSubFunction<T> func)
		{
			auto iter = this->mSubMethodMap.find(name);
			if (iter != this->mSubMethodMap.end())
			{
				return false;
			}
			this->mSubMethodMap.emplace(name, std::make_shared<JsonSubMethod<T>>((T*)this, func));
			return true;
		}
	 private:
		std::unordered_map<std::string, std::shared_ptr<SubMethod>> mSubMethodMap;
	};
#define BIND_SUB_FUNCTION(func) LOG_CHECK_RET_FALSE(this->Bind(GetFunctionName(#func), &func))
}

#endif //SENTRY_REDISSUBSERVICE_H
