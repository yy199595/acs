#pragma once

#include<memory>
#include"Entity/Component/Component.h"
#include"Rpc/Method/MethodRegister.h"
namespace Msg
{
    class Packet;
};
namespace Lua
{
	class LuaModule;
}
namespace Tendo
{
	class ServiceMethod;
	class RpcMethodConfig;
	class RpcServiceConfig;
	class InnerNetTcpClient;
	class RpcService : public Component, public IStart, public IComplete, public ISecondUpdate, public IHotfix
	{
	public:
		RpcService();
	protected:
		bool LateAwake() final;
		virtual bool OnInit() = 0; //注册rpc回调
		virtual void OnStop() { }; //服务关闭调用(协程)
		virtual void OnStart() { }; //服务启动调用(协程)
		virtual void OnSecond(int tick) { } //每秒调用
		virtual void OnComplete() { } //服务器启动成功调用(协程)
	private:
		void Start() final;
		void Complete() final;
		void OnHotFix() final;
		void OnSecondUpdate(int tick) final;
	public:
		void Stop();
		Lua::LuaModule * GetLuaModule() { return this->mLuaModule; }
		const std::string& GetServer() const { return this->mCluster; }
		int Invoke(const std::string& method, const std::shared_ptr<Msg::Packet>& message);
	private:
		int CallLua(const RpcMethodConfig * config, Msg::Packet & message);
		int AwaitCallLua(const RpcMethodConfig * config, Msg::Packet & message);
		int WriterToLua(const RpcMethodConfig * config, Msg::Packet & message);
	protected:
		inline ServiceMethodRegister& GetMethodRegistry() { return this->mMethodRegister; }
	private:
		std::string mCluster;
		Lua::LuaModule * mLuaModule;
		const RpcServiceConfig * mConfig;
		ServiceMethodRegister mMethodRegister;
	};
#define BIND_COMMON_RPC_METHOD(func) LOG_CHECK_RET_FALSE(this->GetMethodRegistry().Bind(GET_FUNC_NAME(#func), &func));
#define BIND_ADDRESS_RPC_METHOD(func) LOG_CHECK_RET_FALSE(this->GetMethodRegistry().BindAddress(GET_FUNC_NAME(#func), &func));
}