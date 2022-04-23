//
// Created by yjz on 2022/1/22.
//

#ifndef SENTRY_HTTPSERVICE_H
#define SENTRY_HTTPSERVICE_H
#include"Json/JsonReader.h"
#include"Component/Component.h"
#include"Method/MethodRegister.h"

namespace Sentry
{
	class HttpService : public Component, public IServiceBase
	{
	 public:
		HttpService() = default;
		virtual ~HttpService() = default;
	 public:
		XCode Get(const std::string & path, std::shared_ptr<Json::Reader> response);
		XCode Post(const std::string & path, Json::Writer & json, std::shared_ptr<Json::Reader> response);
	 protected:
		bool LoadService() final;
		bool LateAwake() override;
		bool IsStartComplete() final { return true; }
		virtual bool OnInitService(HttpServiceRegister & serviceRegister) = 0;
		bool IsStartService() final { return this->mServiceRegister != nullptr;}
	 public:
		void OnAddAddress(const std::string &address) final;
		XCode Invoke(const std::string& name, std::shared_ptr<Json::Reader> request, std::shared_ptr<Json::Writer> response);
	 private:
		std::string mUrl;
		class HttpComponent * mHttpComponent;
		std::shared_ptr<HttpServiceRegister> mServiceRegister;
	};
}
#endif //SENTRY_HTTPSERVICE_H
