//
// Created by yjz on 2022/1/22.
//

#include"Json/JsonWriter.h"
#include"Component/Component.h"
#include"Method/MethodRegister.h"
#ifndef SENTRY_REDISSUBSERVICE_H
#define SENTRY_REDISSUBSERVICE_H

namespace Sentry
{
	class SubService : public Component, public IServiceBase
	{
	 public:
		SubService() = default;
		virtual ~SubService() = default;
	 public:
		void GetSubMethods(std::vector<std::string>& methods);
		bool Invoke(const std::string& func, const Json::Reader & jsonReader);
	public:
		bool Publish(const std::string & func, Json::Writer & jsonWriter);
		bool Publish(const std::string & address, const std::string & func, Json::Writer & jsonWriter);
	protected:
		bool LateAwake() override;
		bool IsStartService() { return this->mServiceRegister != nullptr; }
		virtual bool OnInitService(SubServiceRegister & methodRegister) = 0;
	public:
		bool LoadService() final;
		void OnAddAddress(const std::string &address) final;
	private:
		class RedisComponent * mRedisComponent;
		std::shared_ptr<SubServiceRegister> mServiceRegister;
	};
}

#endif //SENTRY_REDISSUBSERVICE_H
