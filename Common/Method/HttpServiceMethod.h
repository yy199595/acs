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
        HttpServiceMethod(const std::string & name) : mName(name) { }
	 public:
        virtual bool IsLuaMethod() const = 0;
        const std::string & GetName() const { return this->mName; }
		virtual XCode Invoke(const HttpHandlerRequest & request, HttpHandlerResponse & response) = 0;

    private:
        const std::string mName;
	};

	template<typename T>
	class CppHttpServiceMethod : public HttpServiceMethod
	{
	 public:
		CppHttpServiceMethod(const std::string & name, T* o, HttpJsonMethod<T> func)
			: HttpServiceMethod(name), mObj(o), mFunction(std::move(func))
		{
		}

	 public:
        bool IsLuaMethod() const { return false; }
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
