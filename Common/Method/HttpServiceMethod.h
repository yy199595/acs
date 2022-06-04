//
// Created by zmhy0073 on 2021/11/2.
//

#ifndef GAMEKEEPER_HTTPSERVICEMETHOD_H
#define GAMEKEEPER_HTTPSERVICEMETHOD_H
#include"Json/JsonWriter.h"
#include"Json/JsonReader.h"
#include<Util/TimeHelper.h>
#include<Network/Http/Http.h>
#include"Network/Http/HttpAsyncRequest.h"
namespace Sentry
{
	template<typename T>
	using HttpJsonMethod = XCode(T::*)(const HttpHandlerRequest& request, HttpHandlerResponse& response);

	class HttpServiceMethod
	{
	 public:
		virtual XCode Invoke(const HttpHandlerRequest & request, HttpHandlerResponse & response) = 0;
	};

	template<typename T>
	class CppHttpServiceMethod : public HttpServiceMethod
	{
	 public:
		CppHttpServiceMethod(T* o, HttpJsonMethod<T> func)
			: mObj(o), mFunction(std::move(func))
		{
		}

	 public:
		XCode Invoke(const HttpHandlerRequest & request, HttpHandlerResponse & response)
		{
			return (this->mObj->*mFunction)(request, response);
		}

	 private:
		T* mObj;
		HttpJsonMethod<T> mFunction;
	};
}
#endif //GAMEKEEPER_HTTPSERVICEMETHOD_H
