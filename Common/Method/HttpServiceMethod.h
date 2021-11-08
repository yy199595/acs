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
namespace GameKeeper
{
    template<typename T>
    using HttpServiceMethodType = HttpStatus(T::*)(HttpRemoteSession *);

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
			this->mJsonString.clear();
			size_t size = session->ReadFromStream(this->mBuffer, 512);
			while (size > 0)
			{
				this->mJsonString.append(this->mBuffer, size);
				size = session->ReadFromStream(this->mBuffer, 512);
			}
			RapidJsonReader jsonReader;
			HttpJsonContent * jsonWriter = new HttpJsonContent();

			XCode code = jsonReader.TryParse(this->mJsonString)
				? XCode::ParseJsonFailure : (_o->*_func)(jsonReader, *jsonWriter);

			jsonWriter->Add("code", code);
			HttpRequestHandler * requestHandler = session->GetReuqestHandler();
			requestHandler->SetResponseContent(jsonWriter);
			return HttpStatus::OK;
		}
    private:
        T * _o;
		char mBuffer[512];
		std::string mJsonString;
        HttpServiceJsonMethodType<T> _func;
    };
}
#endif //GAMEKEEPER_HTTPSERVICEMETHOD_H
