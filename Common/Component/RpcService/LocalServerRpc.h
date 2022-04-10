//
// Created by mac on 2022/4/6.
//

#ifndef SERVER_LOCALSERVERRPC_H
#define SERVER_LOCALSERVERRPC_H
#include"RpcServiceNode.h"
#include"Method/ServiceMethod.h"
#include"Component/Lua/LuaScriptComponent.h"
namespace Sentry
{
	class LocalServerRpc : public RpcServiceNode, public ILuaRegister
	{
	public:
		LocalServerRpc() = default;
	 protected:
		template<typename T>
		bool Bind(std::string name, ServiceMethodType1<T> func);

		template<typename T>
		bool Bind(std::string name, ServiceMethodType11<T> func);

		template<typename T, typename T1>
		bool Bind(std::string name, ServiceMethodType2<T, T1> func);

		template<typename T, typename T1>
		bool Bind(std::string name, ServiceMethodType22<T, T1> func);

		template<typename T, typename T1, typename T2>
		bool Bind(std::string name, ServiceMethodType3<T, T1, T2> func);

		template<typename T, typename T1, typename T2>
		bool Bind(std::string name, ServiceMethodType33<T, T1, T2> func);

		template<typename T, typename T1>
		bool Bind(std::string name, ServiceMethodType4<T, T1> func);

		template<typename T, typename T1>
		bool Bind(std::string name, ServiceMethodType44<T, T1> func);

		bool AddMethod(std::shared_ptr<ServiceMethod> method);

		std::shared_ptr<ServiceMethod> GetMethod(const std::string& name);

	public:
		void OnLuaRegister(lua_State *lua) final;
		//bool IsStartComplete() { return true; }
		std::shared_ptr<com::Rpc_Response> Invoke(const std::string& method, std::shared_ptr<com::Rpc_Request> request);
	protected:
		std::unordered_map<std::string, std::shared_ptr<ServiceMethod>> mMethodMap;
		std::unordered_map<std::string, std::shared_ptr<ServiceMethod>> mLuaMethodMap;
	};

#define BIND_RPC_FUNCTION(func) LOG_CHECK_RET_FALSE(this->Bind(GetFunctionName(#func), &func))
}

namespace Sentry
{
	template<typename T>
	bool LocalServerRpc::Bind(std::string name, ServiceMethodType1<T> func)
	{
		return this->AddMethod(std::make_shared<ServiceMethod1<T>>(name, (T*)this, func));
	}

	template<typename T>
	bool LocalServerRpc::Bind( std::string name, ServiceMethodType11<T> func)
	{
		return this->AddMethod(std::make_shared<ServiceMethod1<T>>(name, (T*)this, func));
	}

	template<typename T, typename T1>
	bool LocalServerRpc::Bind(std::string name, ServiceMethodType2<T, T1> func)
	{
		return this->AddMethod(std::make_shared<ServiceMethod2<T, T1>>(name, (T*)this, func));
	}

	template<typename T, typename T1>
	bool LocalServerRpc::Bind(std::string name, ServiceMethodType22<T, T1> func)
	{
		return this->AddMethod(std::make_shared<ServiceMethod2<T, T1>>(name, (T*)this, func));
	}

	template<typename T, typename T1, typename T2>
	bool LocalServerRpc::Bind(std::string name, ServiceMethodType3<T, T1, T2> func)
	{
		return this->AddMethod(std::make_shared<ServiceMethod3<T, T1, T2>>(name, (T*)this, func));
	}

	template<typename T, typename T1, typename T2>
	bool LocalServerRpc::Bind(std::string name, ServiceMethodType33<T, T1, T2> func)
	{
		return this->AddMethod(std::make_shared<ServiceMethod3<T, T1, T2>>(name, (T*)this, func));
	}

	template<typename T, typename T1>
	bool LocalServerRpc::Bind(std::string name, ServiceMethodType4<T, T1> func)
	{
		return this->AddMethod(std::make_shared<ServiceMethod4<T, T1>>(name, (T*)this, func));
	}

	template<typename T, typename T1>
	bool LocalServerRpc::Bind(std::string name, ServiceMethodType44<T, T1> func)
	{
		return this->AddMethod(std::make_shared<ServiceMethod4<T, T1>>(name, (T*)this, func));
	}
}
#endif //SERVER_LOCALSERVERRPC_H
