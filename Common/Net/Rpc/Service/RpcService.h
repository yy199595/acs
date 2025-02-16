#pragma once

#include<memory>
#include"Entity/Component/Component.h"
#include"Rpc/Method/MethodRegister.h"
namespace rpc
{
	class Message;
};
namespace Lua
{
	class LuaModule;
}

namespace acs
{
	class ServiceMethod;
	class RpcMethodConfig;
	class RpcService : public Component
	{
	public:
		RpcService();
	protected:
		bool LateAwake() final;
		virtual bool OnInit() = 0; //注册rpc回调
	public:
		Lua::LuaModule * GetLuaModule() { return this->mLuaModule; }
		const std::string& GetServer() const { return this->mCluster; }
		int Invoke(const RpcMethodConfig * config, rpc::Message * message) noexcept;
	private:
		int CallLua(const RpcMethodConfig * config, rpc::Message & message) noexcept;
		int WriterToLua(const RpcMethodConfig * config, rpc::Message & message) noexcept;
		int AwaitCallLua(const RpcMethodConfig * config, rpc::Message & message) noexcept;
	protected:
		inline ServiceMethodRegister& GetMethodRegistry() { return this->mMethodRegister; }
	private:
		std::string mCluster;
		Lua::LuaModule * mLuaModule;
		class ProtoComponent * mProto;
		ServiceMethodRegister mMethodRegister;
	};
#define BIND_PLAYER_RPC_METHOD(func) LOG_CHECK_RET_FALSE(this->GetMethodRegistry().Bind(GET_FUNC_NAME(#func), &func, rpc::Header::player_id ))
#define BIND_SERVER_RPC_METHOD(func) LOG_CHECK_RET_FALSE(this->GetMethodRegistry().Bind(GET_FUNC_NAME(#func), &func, rpc::Header::app_id))
}