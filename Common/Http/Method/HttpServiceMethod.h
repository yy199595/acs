//
// Created by zmhy0073 on 2021/11/2.
//

#ifndef GAMEKEEPER_HTTPSERVICEMETHOD_H
#define GAMEKEEPER_HTTPSERVICEMETHOD_H
#include <utility>

#include"Json/JsonWriter.h"
#include"Json/JsonReader.h"
#include"Time/TimeHelper.h"
#include"Client/Http.h"
#include"Http/HttpRequest.h"
#include"Http/HttpResponse.h"
namespace Sentry
{
	template<typename T>
	using HttpMethod = int(T::*)(const Http::Request& request, Http::Response& response);

    template<typename T>
    using HttpJsonMethod1 = int(T::*)(const Json::Reader & request);

    template<typename T>
    using HttpJsonMethod2 = int(T::*)(const Json::Reader & request, Json::Writer & response);

    template<typename T>
    using HttpJsonMethod3 = int(T::*)(Json::Writer & response);

	class HttpServiceMethod
	{
    public:
        explicit HttpServiceMethod(std::string  name)  : mName(std::move(name)) { }
	 public:
        virtual bool IsLuaMethod() const = 0;
        const std::string & GetName() const { return this->mName; }
		virtual int Invoke(const Http::Request & request, Http::Response & response) = 0;

    private:
        const std::string mName;
	};

	template<typename T>
	class CppHttpServiceMethod : public HttpServiceMethod
	{
	 public:
		CppHttpServiceMethod(const std::string & name, T* o, HttpMethod<T> func)
			: HttpServiceMethod(name), mObj(o), mFunction(std::move(func))
		{
		}

	 public:
        bool IsLuaMethod() const { return false; }
		int Invoke(const Http::Request & request, Http::Response & response)
		{
			return (this->mObj->*mFunction)(request, response);
		}

	 private:
		T* mObj;
		HttpMethod<T> mFunction;
	};

    template<typename T>
    class JsonHttpServiceMethod1 : public HttpServiceMethod
    {
    public:
        JsonHttpServiceMethod1(const std::string & name, T* o, HttpJsonMethod1<T> func)
            : HttpServiceMethod(name), mObj(o), mFunction(std::move(func))
        {
        }

    public:
        bool IsLuaMethod() const { return false; }
        int Invoke(const Http::Request& request, Http::Response& response)
        {
            int code;
            std::unique_ptr<Json::Writer> document(new Json::Writer());
            std::unique_ptr<Json::Reader> document1(new Json::Reader());
            if (!request.WriteDocument(document1.get()))
            {
                document->Add("error").Add("parse json error");
                document->Add("code").Add((int)XCode::ParseJsonFailure);
                response.Json(HttpStatus::OK, *document);
                return XCode::ParseJsonFailure;
            }
            try
            {
				code = (this->mObj->*mFunction)(*document1);
                document->Add("code").Add((int)code);
            }
            catch (std::exception& e)
            {
                code = XCode::ThrowError;
                document->Add("error").Add(e.what());
            }
            document->Add("code").Add((int)code);
            response.Json(HttpStatus::OK, *document);
            return code;
        }

    private:
        T* mObj;
        HttpJsonMethod1<T> mFunction;
    };

    template<typename T>
    class JsonHttpServiceMethod2 : public HttpServiceMethod
    {
    public:
        JsonHttpServiceMethod2(const std::string & name, T* o, HttpJsonMethod2<T> func)
            : HttpServiceMethod(name), mObj(o), mFunction(std::move(func))
        {
        }

    public:
        bool IsLuaMethod() const final { return false; }
        int Invoke(const Http::Request& request, Http::Response& response) final
        {
            int code = XCode::Failure;
            std::unique_ptr<Json::Reader> document1(new Json::Reader());
            std::unique_ptr<Json::Writer> document2(new Json::Writer());
            if (!request.WriteDocument(document1.get()))
            {
                document2->Add("error").Add("parse json error");
                document2->Add("code").Add((int)XCode::ParseJsonFailure);
                response.Json(HttpStatus::OK, *document2);
                return XCode::ParseJsonFailure;
            }
            try
            {
                code = (this->mObj->*mFunction)(*document1, *document2);
            }
            catch (std::exception& e)
            {
                code = XCode::ThrowError;
                document2->Add("error").Add(e.what());
            }
            document2->Add("code").Add((int)code);
            response.Json(HttpStatus::OK, *document2);
            return code;
        }

    private:
        T* mObj;
        HttpJsonMethod2<T> mFunction;
    };

    template<typename T>
    class JsonHttpServiceMethod3 : public HttpServiceMethod
    {
    public:
        JsonHttpServiceMethod3(const std::string & name, T* o, HttpJsonMethod3<T> func)
            : HttpServiceMethod(name), mObj(o), mFunction(std::move(func))
        {
        }

    public:
        bool IsLuaMethod() const { return false; }
        int Invoke(const Http::Request & request, Http::Response & response)
        {
            int code = XCode::Failure;
            std::unique_ptr<Json::Writer> document2(new Json::Writer());
            try
            {
                code = (this->mObj->*mFunction)(*document2);               
            }
            catch(std::exception & e)
            {
                code = XCode::ThrowError;
                document2->Add("error").Add(e.what());
            }
            document2->Add("code").Add((int)code);
            response.Json(HttpStatus::OK, *document2);
            return code;
        }

    private:
        T* mObj;
        HttpJsonMethod3<T> mFunction;
    };


}
#endif //GAMEKEEPER_HTTPSERVICEMETHOD_H
