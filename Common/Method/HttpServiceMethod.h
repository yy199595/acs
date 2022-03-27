//
// Created by zmhy0073 on 2021/11/2.
//

#ifndef GAMEKEEPER_HTTPSERVICEMETHOD_H
#define GAMEKEEPER_HTTPSERVICEMETHOD_H
#include"Json/JsonWriter.h"
#include"Json/JsonReader.h"
#include<Util/TimeHelper.h>
#include<Network/Http/Http.h>
namespace Sentry
{
	template<typename T>
	using HttpJsonMethod1 = XCode(T::*)(Json::Writer& response);

	template<typename T>
	using HttpJsonMethod2 = XCode(T::*)(const Json::Reader& request, Json::Writer& response);

	class HttpServiceMethod
	{
	 public:
		virtual XCode Invoke(const std::shared_ptr<Json::Reader> request, std::shared_ptr<Json::Writer> response) = 0;
	};

	template<typename T>
	class HttpServiceJsonMethod1 : public HttpServiceMethod
	{
	 public:
		HttpServiceJsonMethod1(T* o, HttpJsonMethod1<T> func)
			: mObj(o), mFunction(std::move(func))
		{
		}

	 public:
		XCode Invoke(const std::shared_ptr<Json::Reader> request, std::shared_ptr<Json::Writer> response)
		{
			return (this->mObj->*mFunction)(*response);
		}

	 private:
		T* mObj;
		HttpJsonMethod1<T> mFunction;
	};

	template<typename T>
	class HttpServiceJsonMethod2 : public HttpServiceMethod
	{
	 public:
		HttpServiceJsonMethod2(T* o, HttpJsonMethod2<T> func)
			: mObj(o), mFunction(std::move(func))
		{
		}

	 public:
		XCode Invoke(std::shared_ptr<Json::Reader> request, std::shared_ptr<Json::Writer> response)
		{
			return (this->mObj->*mFunction)(*request, *response);
		}

	 private:
		T* mObj;
		HttpJsonMethod2<T> mFunction;
	};
}
#endif //GAMEKEEPER_HTTPSERVICEMETHOD_H
