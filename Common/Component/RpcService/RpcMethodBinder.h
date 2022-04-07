//
// Created by mac on 2022/4/7.
//

#ifndef SERVER_RPCMETHODBINDER_H
#define SERVER_RPCMETHODBINDER_H
#include"Method/SubMethod.h"
#include"Method/ServiceMethod.h"
namespace Sentry
{
	class RpcMethodBinder
	{
	protected:
		template<typename T>
		bool Bind(std::string service, std::string name, ServiceMethodType1<T> func)
		{
			return this->AddMethod(service,std::make_shared<ServiceMethod1<T>>(name, (T*)this, func));
		}

		template<typename T>
		bool Bind(std::string service, std::string name, ServiceMethodType11<T> func)
		{
			return this->AddMethod(service,std::make_shared<ServiceMethod1<T>>(name, (T*)this, func));
		}

		template<typename T, typename T1>
		bool Bind(std::string service, std::string name, ServiceMethodType2<T, T1> func)
		{
			return this->AddMethod(service,std::make_shared<ServiceMethod2<T, T1>>(name, (T*)this, func));
		}

		template<typename T, typename T1>
		bool Bind(std::string service, std::string name, ServiceMethodType22<T, T1> func)
		{
			return this->AddMethod(service,std::make_shared<ServiceMethod2<T, T1>>(name, (T*)this, func));
		}

		template<typename T, typename T1, typename T2>
		bool Bind(std::string service, std::string name, ServiceMethodType3<T, T1, T2> func)
		{
			return this->AddMethod(service,std::make_shared<ServiceMethod3<T, T1, T2>>(name, (T*)this, func));
		}

		template<typename T, typename T1, typename T2>
		bool Bind(std::string service, std::string name, ServiceMethodType33<T, T1, T2> func)
		{
			return this->AddMethod(service,std::make_shared<ServiceMethod3<T, T1, T2>>(name, (T*)this, func));
		}

		template<typename T, typename T1>
		bool Bind(std::string service, std::string name, ServiceMethodType4<T, T1> func)
		{
			return this->AddMethod(service,std::make_shared<ServiceMethod4<T, T1>>(name, (T*)this, func));
		}

		template<typename T, typename T1>
		bool Bind(std::string service, std::string name, ServiceMethodType44<T, T1> func)
		{
			return this->AddMethod(service,std::make_shared<ServiceMethod4<T, T1>>(name, (T*)this, func));
		}

	public:
		bool AddMethod(std::string service, std::shared_ptr<ServiceMethod> method);

	protected:
		std::shared_ptr<ServiceMethod> GetMethod(const std::string& name);
		std::unordered_map<std::string, std::shared_ptr<ServiceMethod>> mMethodMap;
		std::unordered_map<std::string, std::shared_ptr<ServiceMethod>> mLuaMethodMap;
	};
}

#endif //SERVER_RPCMETHODBINDER_H
