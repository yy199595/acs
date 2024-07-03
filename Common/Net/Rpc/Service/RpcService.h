#pragma once

#include<memory>
#include"Entity/Component/Component.h"
#include"Rpc/Method/MethodRegister.h"
namespace rpc
{
    class Packet;
};
namespace Lua
{
	class LuaModule;
}

namespace joke
{
	class ServiceMethod;
	class RpcMethodConfig;
	class RpcService : public Component, public IStart, public IComplete, public IAppStop
	{
	public:
		RpcService();
	protected:
		bool LateAwake() final;
		virtual bool OnInit() = 0; //注册rpc回调
		virtual void OnStop() { }; //服务关闭调用(协程)
		virtual void OnStart() { }; //服务启动调用(协程)
		virtual void OnComplete() { } //服务器启动成功调用(协程)
	private:
		void Start() final;
		void Complete() final;
		void OnAppStop() final;
	public:
		Lua::LuaModule * GetLuaModule() { return this->mLuaModule; }
		const std::string& GetServer() const { return this->mCluster; }
		int Invoke(const RpcMethodConfig * config, rpc::Packet * message);
	private:
		int CallLua(const RpcMethodConfig * config, rpc::Packet & message);
		int AwaitCallLua(const RpcMethodConfig * config, rpc::Packet & message);
		int WriterToLua(const RpcMethodConfig * config, rpc::Packet & message);
	protected:
		inline ServiceMethodRegister& GetMethodRegistry() { return this->mMethodRegister; }
	private:
		std::string mCluster;
		Lua::LuaModule * mLuaModule;
		class ProtoComponent * mProto;
		ServiceMethodRegister mMethodRegister;
	};
#define BIND_PLAYER_RPC_METHOD(func) LOG_CHECK_RET_FALSE(this->GetMethodRegistry().Bind(GET_FUNC_NAME(#func), &func, "id"))
#define BIND_SERVER_RPC_METHOD(func) LOG_CHECK_RET_FALSE(this->GetMethodRegistry().Bind(GET_FUNC_NAME(#func), &func, "app"))
}