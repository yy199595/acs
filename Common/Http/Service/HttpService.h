//
// Created by yjz on 2022/1/22.
//

#ifndef SENTRY_HTTPSERVICE_H
#define SENTRY_HTTPSERVICE_H
#include"Entity/Component/Component.h"
#include"Rpc/Method/MethodRegister.h"
#include"Server//Config/ServiceConfig.h"
namespace Json
{
    class Writer;
}
namespace Http
{
    class Request;
    class DataResponse;
}
namespace Tendo
{
    class HttpService : public Component,
						public IService<Http::Request, Http::DataResponse>
	{
	 public:
		HttpService();
		virtual ~HttpService() = default;
	 protected:
		virtual bool OnInit() = 0;
		virtual void OnClose() {  }
		virtual void OnStart() { }
	protected:
		bool Init() final;
		void Start() final;
		void Close() final;
		bool LateAwake() final;
		bool LoadFromLua() final;
		HttpServiceRegister & GetRegister() { return this->mServiceRegister;}
	 private:
		HttpServiceRegister mServiceRegister;
		class LuaScriptComponent * mLuaComponent;
	};
}
#endif //SENTRY_HTTPSERVICE_H
