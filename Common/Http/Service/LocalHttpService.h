//
// Created by yjz on 2022/1/22.
//

#ifndef SENTRY_HTTPSERVICE_H
#define SENTRY_HTTPSERVICE_H
#include"Json/JsonReader.h"
#include"Component/Component.h"
#include"Method/MethodRegister.h"
#include"Config/ServiceConfig.h"
#include"Http/HttpRequest.h"
#include"Http/HttpResponse.h"
namespace Sentry
{
    class LocalHttpService : public Component, public IService<Http::Request, Http::Response>
	{
	 public:
		LocalHttpService() = default;
		virtual ~LocalHttpService() = default;
	 protected:
        virtual bool OnCloseService() { return true; };
		virtual bool OnStartService(HttpServiceRegister & serviceRegister) = 0;
	 public:
        bool Start() final;
        bool Close() final;
		bool LoadFromLua() final;
		bool LateAwake() override;
		bool IsStartService() final { return this->mServiceRegister != nullptr;}
		XCode Invoke(const std::string & name, std::shared_ptr<Http::Request>, std::shared_ptr<Http::Response>) final;
	 private:
		std::shared_ptr<HttpServiceRegister> mServiceRegister;
	};
}
#endif //SENTRY_HTTPSERVICE_H
