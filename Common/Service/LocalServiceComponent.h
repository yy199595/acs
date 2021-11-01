#pragma once

#include "ServiceComponent.h"
#include <Method/ServiceMethod.h>

namespace GameKeeper
{

	inline std::string GetFunctionName(const std::string func)
	{
		size_t pos = func.find("::");
		return func.substr(pos + 2);
	}

    class LocalServiceComponent : public ServiceComponent
    {
    public:
        LocalServiceComponent();

        virtual ~LocalServiceComponent() {}

    public:
		const std::string &GetServiceName()final { return this->GetTypeName(); }
	protected:

		template<typename T>
		bool Bind(std::string name, ServiceMethodType1<T> func) {
			return this->AddMethod(new ServiceMethod1<T>(name, (T*)this, func));
		}

		template<typename T>
		bool Bind(std::string name, ServiceMethodType11<T> func) {
			return this->AddMethod(new ServiceMethod1<T>(name, (T*)this, func));
		}

		template<typename T, typename T1>
		bool Bind(std::string name, ServiceMethodType2<T, T1> func) {
			return this->AddMethod(new ServiceMethod2<T, T1>(name, (T*)this, func));
		}

		template<typename T, typename T1>
		bool Bind(std::string name, ServiceMethodType22<T, T1> func) {
			return this->AddMethod(new ServiceMethod2<T, T1>(name, (T*)this, func));
		}

		template<typename T, typename T1, typename T2>
		bool Bind(std::string name, ServiceMethodType3<T, T1, T2> func) {
			return this->AddMethod(new ServiceMethod3<T, T1, T2>(name, (T*)this, func));
		}

		template<typename T, typename T1, typename T2>
		bool Bind(std::string name, ServiceMethodType33<T, T1, T2> func) {
			return this->AddMethod(new ServiceMethod3<T, T1, T2>(name, (T*)this, func));
		}

		template<typename T, typename T1>
		bool Bind(std::string name, ServiceMethodType4<T, T1> func) {
			return this->AddMethod(new ServiceMethod4<T, T1>(name, (T*)this, func));
		}

		template<typename T, typename T1>
		bool Bind(std::string name, ServiceMethodType44<T, T1> func) {
			return this->AddMethod(new ServiceMethod4<T, T1>(name, (T*)this, func));
		}
    };
#define __add_method(func) SayNoAssertRetFalse_F(this->Bind(GetFunctionName(#func), &func))
}// namespace GameKeeper