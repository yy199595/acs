//
// Created by yjz on 2022/1/22.
//

#ifndef SENTRY_HTTPSERVICE_H
#define SENTRY_HTTPSERVICE_H
#include"Entity/Component/Component.h"
#include"Rpc/Method/MethodRegister.h"
#include"Rpc/Config/ServiceConfig.h"
namespace Json
{
    class Writer;
}
namespace Http
{
    class Request;
    class DataResponse;
}
namespace Lua
{
	class LuaModule;
}
namespace Tendo
{
	class HttpServiceConfig;
    class HttpService : public Component,
						public IService<Http::Request, Http::DataResponse>,
						public IStart, public IDestroy, public ISecondUpdate, public IComplete, public IHotfix
	{
	 public:
		HttpService();
		virtual ~HttpService() = default;
	 protected:
		virtual bool OnInit() = 0;
		virtual void OnStop() { }
		virtual void OnStart() { }
		virtual void OnSecond(int tick) { }
		virtual void OnComplete() { }
	protected:
		void Start() final;
		bool OnHotFix() final;
		void Complete() final;
		void OnDestroy() final;
		bool LateAwake() final;
		void OnSecondUpdate(int tick) final;
		HttpServiceRegister & GetRegister() { return this->mServiceRegister;}
	public:
		Lua::LuaModule * GetLuaModule() { return this->mLuaModule; }
		int Invoke(const std::string &, const std::shared_ptr<Http::Request> &, std::shared_ptr<Http::DataResponse> &) final;
	private:
		int CallLua(const std::string & method, const Http::Request & request, Http::DataResponse & response);
		int AwaitCallLua(const std::string & method, const Http::Request & request, Http::DataResponse & response);
	private:
		Lua::LuaModule * mLuaModule;
		const HttpServiceConfig * mConfig;
		HttpServiceRegister mServiceRegister;
	};
}
#define BIND_COMMON_HTTP_METHOD(func) this->GetRegister().Bind(GET_FUNC_NAME(#func), &func)
#endif //SENTRY_HTTPSERVICE_H
