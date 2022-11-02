//
// Created by zmhy0073 on 2021/11/2.
//

#ifndef GAMEKEEPER_HTTPSERVICEMETHOD_H
#define GAMEKEEPER_HTTPSERVICEMETHOD_H
#include"Json/JsonWriter.h"
#include"Json/JsonReader.h"
#include"Time/TimeHelper.h"
#include"Client/Http.h"
#include"Http/HttpRequest.h"
#include"Http/HttpResponse.h"
namespace Sentry
{
	template<typename T>
	using HttpMethod = XCode(T::*)(const Http::Request& request, Http::Response& response);

    template<typename T>
    using HttpJsonMethod1 = XCode(T::*)(const Json::Reader & request);

    template<typename T>
    using HttpJsonMethod2 = XCode(T::*)(const Json::Reader & request, Json::Document & response);

    template<typename T>
    using HttpJsonMethod3 = XCode(T::*)(Json::Document & response);

	class HttpServiceMethod
	{
    public:
        HttpServiceMethod(const std::string & name) : mName(name) { }
	 public:
        virtual bool IsLuaMethod() const = 0;
        const std::string & GetName() const { return this->mName; }
		virtual XCode Invoke(const Http::Request & request, Http::Response & response) = 0;

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
		XCode Invoke(const Http::Request & request, Http::Response & response)
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
        XCode Invoke(const Http::Request & request, Http::Response & response)
        {
            std::unique_ptr<Json::Reader> document1(new Json::Reader());
            std::unique_ptr<Json::Document> document(new Json::Document());
            if(!request.WriteDocument(document1.get()))
            {
                document->Add("error", "parse json error");
                document->Add("code", (int)XCode::ParseJsonFailure);
                return XCode::ParseJsonFailure;
            }
            XCode code = XCode::Failure;
            try
            {
                code = (this->mObj->*mFunction)(*document1);
            }
            catch (std::exception & e)
            {
                code = XCode::ThrowError;
                document->Add("error", e.what());
            }
            document->Add("code", (int)code);
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
        bool IsLuaMethod() const { return false; }
        XCode Invoke(const Http::Request & request, Http::Response & response)
        {
            std::unique_ptr<Json::Reader> document1(new Json::Reader());
            std::unique_ptr<Json::Document> document(new Json::Document());
            if(!request.WriteDocument(document1.get()))
            {
                document->Add("error", "parse json error");
                document->Add("code", (int)XCode::ParseJsonFailure);
                return XCode::ParseJsonFailure;
            }
            XCode code = XCode::Failure;
            try
            {
                std::unique_ptr<Json::Document> document2(new Json::Document());
                code = (this->mObj->*mFunction)(*document1, *document2);
                if(code == XCode::Successful)
                {
                    document->Add("data", *document2);
                }
            }
            catch(std::exception & e)
            {
                code = XCode::ThrowError;
                document->Add("error", e.what());
            }
            document->Add("code", (int)code);
            response.Json(HttpStatus::OK, *document);
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
        XCode Invoke(const Http::Request & request, Http::Response & response)
        {
            XCode code = XCode::Failure;
            std::unique_ptr<Json::Document> document(new Json::Document());
            try
            {
                std::unique_ptr<Json::Document> document2(new Json::Document());
                code = (this->mObj->*mFunction)(*document2);
                if(code == XCode::Successful)
                {
                    document->Add("data", *document2);
                }
            }
            catch(std::exception & e)
            {
                code = XCode::ThrowError;
                document->Add("error", e.what());
            }
            document->Add("code", (int)code);
            response.Json(HttpStatus::OK, *document);
            return code;
        }

    private:
        T* mObj;
        HttpJsonMethod3<T> mFunction;
    };


}
#endif //GAMEKEEPER_HTTPSERVICEMETHOD_H
