//
// Created by yjz on 2022/1/22.
//

#ifndef SENTRY_HTTPSERVICE_H
#define SENTRY_HTTPSERVICE_H
#include"Component/Component.h"
#include"Method/MethodRegister.h"
#include"Config/ServiceConfig.h"
namespace Json
{
    class Writer;
}
namespace Http
{
    class Request;
    class Response;
}
namespace Sentry
{
    class LocalHttpService : public Component,
                             public IService<Http::Request, Http::Response>, public IServerRecord
	{
	 public:
		LocalHttpService();
		virtual ~LocalHttpService() = default;
	 protected:
		virtual bool OnInit() = 0;
		virtual bool OnClose() { return true; }
		virtual bool OnStart() { return true; }
	 public:
		bool Init() final;
        bool Start() final;
        bool Close() final;
		bool LoadFromLua() final;
		bool LateAwake() final;
        void OnRecord(Json::Writer&document) final;
		bool IsStartService() final { return true;}
		int Invoke(const std::string & name, std::shared_ptr<Http::Request>, std::shared_ptr<Http::Response>) final;
	protected:
		HttpServiceRegister & GetRegister() { return this->mServiceRegister;}
	 private:
        unsigned int mSumCount;
        unsigned int mWaitCount;
		HttpServiceRegister mServiceRegister;
	};
}
#endif //SENTRY_HTTPSERVICE_H
