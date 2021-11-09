//
// Created by zmhy0073 on 2021/11/2.
//

#ifndef GAMEKEEPER_HTTPSERVICEMETHOD_H
#define GAMEKEEPER_HTTPSERVICEMETHOD_H
#include <Network/Http/Http.h>
#include <Util/JsonHelper.h>
#include <Network/Http/Content/HttpReadContent.h>
#include <Network/Http/HttpRemoteSession.h>
#include <Network/Http/Response/HttpRequestHandler.h>
#include<Network/Http/Content/HttpWriteContent.h>
#include <Util/TimeHelper.h>
namespace GameKeeper
{
    template<typename T>
    using HttpServiceMethodType = HttpStatus(T::*)(HttpRemoteSession *);

    template<typename T>
    using HttpServiceJsonMethodRequestType = XCode(T::*)(RapidJsonWriter & response);

    template<typename T>
    using HttpServiceJsonMethodType = XCode(T::*)(const RapidJsonReader & request, RapidJsonWriter & response);

    class HttpServiceMethod
    {
    public:
        virtual HttpStatus Invoke(HttpRemoteSession *session) = 0;
    };

    template<typename T>
    class HttpServiceMethod1 : public HttpServiceMethod
    {
    public:
        HttpServiceMethod1(T *o, HttpServiceMethodType<T> func)
                : _o(o), _func(func){}

    public:
		HttpStatus Invoke(HttpRemoteSession * session) override
        {
            return (_o->*_func)(session);
        }
    private:
        T *_o;
        HttpServiceMethodType<T> _func;
    };
    template<typename T>
    class HttpServiceJsonMethod : public HttpServiceMethod
    {
    public:
        HttpServiceJsonMethod(T * o, HttpServiceJsonMethodType<T> func)
                : _o(o), _func(func){}

    public:
		HttpStatus Invoke(HttpRemoteSession *session) override
        {
            const std::string & json = session->GetReuqestHandler()->GetParamater();

            RapidJsonReader jsonReader;
            auto jsonWriter = new HttpJsonContent();
            XCode code = !jsonReader.TryParse(json)
                         ? XCode::ParseJsonFailure : (_o->*_func)(jsonReader, *jsonWriter);

            jsonWriter->Add("code", code);
            HttpRequestHandler *requestHandler = session->GetReuqestHandler();
            requestHandler->SetResponseContent(jsonWriter);
            return HttpStatus::OK;
        }
    private:
        T * _o;
        HttpServiceJsonMethodType<T> _func;
    };



    template<typename T>
    class HttpServiceJsonRequestMethod : public HttpServiceMethod
    {
    public:
        HttpServiceJsonRequestMethod(T * o, HttpServiceJsonMethodRequestType<T> func)
                : _o(o), _func(func){}

    public:
        HttpStatus Invoke(HttpRemoteSession *session) override
        {
            auto jsonWriter = new HttpJsonContent();
            XCode code = (_o->*_func)(*jsonWriter);

            jsonWriter->Add("code", code);
            HttpRequestHandler *requestHandler = session->GetReuqestHandler();
            requestHandler->SetResponseContent(jsonWriter);
            return HttpStatus::OK;
        }
    private:
        T * _o;
        HttpServiceJsonMethodRequestType<T> _func;
    };
}
#endif //GAMEKEEPER_HTTPSERVICEMETHOD_H
