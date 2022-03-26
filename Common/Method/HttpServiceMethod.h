//
// Created by zmhy0073 on 2021/11/2.
//

#ifndef GAMEKEEPER_HTTPSERVICEMETHOD_H
#define GAMEKEEPER_HTTPSERVICEMETHOD_H
#include<Network/Http/Http.h>
#include<Util/JsonHelper.h>
#include<Util/TimeHelper.h>
namespace Sentry
{
    template<typename T>
    using HttpJsonMethod1 = XCode(T::*)(RapidJsonWriter & response);

    template<typename T>
    using HttpJsonMethod2 = XCode(T::*)(const RapidJsonReader & request, RapidJsonWriter & response);

    class HttpServiceMethod
    {
    public:
        virtual XCode Invoke(const std::shared_ptr<RapidJsonReader> request, std::shared_ptr<RapidJsonWriter> response) = 0;
    };

    template<typename T>
    class HttpServiceJsonMethod1 : public HttpServiceMethod
    {
    public:
        HttpServiceJsonMethod1(T *o, HttpJsonMethod1<T> func)
            : mObj(o), mFunction(std::move(func)) { }

    public:
        XCode Invoke(const std::shared_ptr<RapidJsonReader> request, std::shared_ptr<RapidJsonWriter> response)
        {
           return (this->mObj->*mFunction)(*response);
        }

    private:
        T * mObj;
        HttpJsonMethod1<T> mFunction;
    };

    template<typename T>
    class HttpServiceJsonMethod2 : public HttpServiceMethod
    {
    public:
        HttpServiceJsonMethod2(T *o, HttpJsonMethod2<T> func)
            : mObj(o), mFunction(std::move(func)) { }

    public:
        XCode Invoke(std::shared_ptr<RapidJsonReader> request, std::shared_ptr<RapidJsonWriter> response)
        {
            return (this->mObj->*mFunction)(*request, *response);
        }

    private:
        T * mObj;
        HttpJsonMethod2<T> mFunction;
    };

}
#endif //GAMEKEEPER_HTTPSERVICEMETHOD_H
