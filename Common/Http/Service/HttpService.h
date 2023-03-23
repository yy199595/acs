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
    class HttpService : public Component,
						public IService<Http::Request, Http::Response>
	{
	 public:
		HttpService();
		virtual ~HttpService() = default;
	 protected:
		virtual bool OnInit() = 0;
		virtual bool OnClose() { return true; }
		virtual bool OnStart() { return true; }
	protected:
		bool Init() final;
		bool Start() final;
		bool Close() final;
		bool LateAwake() final;
		bool LoadFromLua() final;
		HttpServiceRegister & GetRegister() { return this->mServiceRegister;}
	 private:
		HttpServiceRegister mServiceRegister;
		class LuaScriptComponent * mLuaComponent;
	};
}
#endif //SENTRY_HTTPSERVICE_H
