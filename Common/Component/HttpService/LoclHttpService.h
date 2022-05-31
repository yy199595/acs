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
	class LoclHttpService : public Component, public IService<Json::Reader, Json::Writer>
	{
	 public:
		LoclHttpService() = default;
		virtual ~LoclHttpService() = default;
	 protected:
		bool StartService() final;
		bool CloseService() final;
		bool IsStartComplete() final { return true; }
		virtual bool OnStartService(HttpServiceRegister & serviceRegister) = 0;
	 public:
		bool IsStartService() final { return this->mServiceRegister != nullptr;}
		XCode Invoke(const std::string& name, std::shared_ptr<Json::Reader> request, std::shared_ptr<Json::Writer> response) final;
	 private:
		std::shared_ptr<HttpServiceRegister> mServiceRegister;
	};
}
#endif //SENTRY_HTTPSERVICE_H
