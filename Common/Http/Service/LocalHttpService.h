//
// Created by yjz on 2022/1/22.
//

#ifndef SENTRY_HTTPSERVICE_H
#define SENTRY_HTTPSERVICE_H
#include"Json/JsonReader.h"
#include"Component/Component.h"
#include"Method/MethodRegister.h"
#include"Config/ServiceConfig.h"
#include"Client/HttpAsyncRequest.h"
namespace Sentry
{
	class LocalHttpService : public Component,
		public IService<HttpHandlerRequest, HttpHandlerResponse>
	{
	 public:
		LocalHttpService();
		virtual ~LocalHttpService() = default;
	 protected:
		bool Start() final;
		bool Close() final;
		bool IsStartComplete() final { return true; }
		bool LoadConfig(const rapidjson::Value & json) final;
        virtual bool OnCloseService() { return true; };
		virtual bool OnStartService(HttpServiceRegister & serviceRegister) = 0;
	 public:
		bool IsStartService() final { return this->mServiceRegister != nullptr;}
		const HttpServiceConfig & GetServiceConfig() const { return *this->mConfig; }
		XCode Invoke(const std::string & name, std::shared_ptr<HttpHandlerRequest>, std::shared_ptr<HttpHandlerResponse>) final;
	 private:
		HttpServiceConfig * mConfig;
		std::shared_ptr<HttpServiceRegister> mServiceRegister;
	};
}
#endif //SENTRY_HTTPSERVICE_H
