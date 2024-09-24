//
// Created by yjz on 2022/1/22.
//

#ifndef APP_HTTPSERVICE_H
#define APP_HTTPSERVICE_H
#include<unordered_map>
#include"Entity/Component/Component.h"
#include"Rpc/Method/MethodRegister.h"
#include"Rpc/Config/ServiceConfig.h"
namespace json
{
    class Writer;
}
namespace http
{
    class Request;
    class Response;
}
namespace Lua
{
	class LuaModule;
}
namespace acs
{
    class HttpService : public Component, public IStart, public IDestroy, public IComplete
	{
	 public:
		HttpService();
	 protected:
		virtual bool OnInit() = 0;
		virtual void OnStop() { }
		virtual void OnStart() { }
		virtual void OnComplete() { }
	protected:
		void Start() final;
		void Complete() final;
		void OnDestroy() final;
		bool LateAwake() final;
		HttpServiceRegister & GetRegister() { return this->mServiceRegister;}
	public:
		Lua::LuaModule * GetLuaModule() { return this->mLuaModule; }
		int Invoke(const HttpMethodConfig * config, const http::Request &, http::Response &);
	private:
		int CallLua(const std::string & method, const http::Request & request, http::Response & response);
		int AwaitCallLua(const std::string & method, const http::Request & request, http::Response & response);
	private:
		Lua::LuaModule * mLuaModule;
		HttpServiceRegister mServiceRegister;
	};
}
#define BIND_COMMON_HTTP_METHOD(func) this->GetRegister().Bind(GET_FUNC_NAME(#func), &func)
#endif //APP_HTTPSERVICE_H
