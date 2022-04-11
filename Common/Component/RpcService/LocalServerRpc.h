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
	class ServiceMethodRegister
	{
	public:
		ServiceMethodRegister(const std::string & name);
	public:
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

	public:
		bool LoadLuaMethod(lua_State *lua);
		bool AddMethod(std::shared_ptr<ServiceMethod> method);
		std::shared_ptr<ServiceMethod> GetMethod(const std::string& name);


	private:
		const std::string mService;
		std::unordered_map<std::string, std::shared_ptr<ServiceMethod>> mMethodMap;
		std::unordered_map<std::string, std::shared_ptr<ServiceMethod>> mLuaMethodMap;
	};

	template<typename T>
	bool ServiceMethodRegister::Bind(std::string name, ServiceMethodType1<T> func)
	{
		return this->AddMethod(std::make_shared<ServiceMethod1<T>>(name, (T*)this, func));
	}

	template<typename T>
	bool ServiceMethodRegister::Bind( std::string name, ServiceMethodType11<T> func)
	{
		return this->AddMethod(std::make_shared<ServiceMethod1<T>>(name, (T*)this, func));
	}

	template<typename T, typename T1>
	bool ServiceMethodRegister::Bind(std::string name, ServiceMethodType2<T, T1> func)
	{
		return this->AddMethod(std::make_shared<ServiceMethod2<T, T1>>(name, (T*)this, func));
	}

	template<typename T, typename T1>
	bool ServiceMethodRegister::Bind(std::string name, ServiceMethodType22<T, T1> func)
	{
		return this->AddMethod(std::make_shared<ServiceMethod2<T, T1>>(name, (T*)this, func));
	}

	template<typename T, typename T1, typename T2>
	bool ServiceMethodRegister::Bind(std::string name, ServiceMethodType3<T, T1, T2> func)
	{
		return this->AddMethod(std::make_shared<ServiceMethod3<T, T1, T2>>(name, (T*)this, func));
	}

	template<typename T, typename T1, typename T2>
	bool ServiceMethodRegister::Bind(std::string name, ServiceMethodType33<T, T1, T2> func)
	{
		return this->AddMethod(std::make_shared<ServiceMethod3<T, T1, T2>>(name, (T*)this, func));
	}

	template<typename T, typename T1>
	bool ServiceMethodRegister::Bind(std::string name, ServiceMethodType4<T, T1> func)
	{
		return this->AddMethod(std::make_shared<ServiceMethod4<T, T1>>(name, (T*)this, func));
	}

	template<typename T, typename T1>
	bool ServiceMethodRegister::Bind(std::string name, ServiceMethodType44<T, T1> func)
	{
		return this->AddMethod(std::make_shared<ServiceMethod4<T, T1>>(name, (T*)this, func));
	}
}


namespace Sentry
{
	class LocalServerRpc : public RpcServiceNode
	{
	public:
		LocalServerRpc() = default;

	protected:
		virtual bool OnInitService(ServiceMethodRegister & methodRegister) = 0;
	public:
		bool LoadServiceMethod();
		bool IsStartService() const { return this->mMethodRegister != nullptr; }
		std::shared_ptr<com::Rpc_Response> Invoke(const std::string& method, std::shared_ptr<com::Rpc_Request> request);
	protected:
		std::shared_ptr<ServiceMethodRegister> mMethodRegister;
	};
}
#endif //SERVER_LOCALSERVERRPC_H
