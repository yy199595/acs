//
// Created by 64658 on 2021/8/5.
//
#include <Core/App.h>
#include <Thread/TaskThread.h>
#include "HttpClientComponent.h"
#include "TaskComponent.h"
#include <Coroutine/CoroutineComponent.h>

namespace Sentry
{
    bool HttpUrlHelper::TryParse(const std::string & url, std::string & host, std::string & port, std::string & path)
    {
        const static std::string http = "http://";
        const static std::string https = "https://";

        std::string content;
        size_t pos = url.find(http);
        if (pos != std::string::npos)
        {
            content = url.substr(http.size());
        }
        else
        {
            pos = url.find(https);
            if (pos != std::string::npos)
            {
                content = url.substr(https.size());
            }
        }
        if (content.empty())
        {
            return false;
        }

        port = "80";
        size_t pos1 = content.find(':');
        size_t pos2 = content.find('/');
        if(pos2 != std::string::npos)
        {
            path = content.substr(pos2);
            host = content.substr(0, pos2);
            if(pos1 != std::string::npos)
            {
                host = content.substr(0, pos1);
                port = content.substr(pos1 + 1, pos2 - pos1 - 1);
            }
        }
        else
        {
            path = "/";
            host = content;
            if(pos1 != std::string::npos)
            {
                host = content.substr(0, pos1);
                port = content.substr(pos1 + 1, pos2 - pos1 - 1);
            }
        }
        return true;
    }
}
namespace Sentry
{
    HttpRequestTask::HttpRequestTask(const std::string url, AsioContext & io)
        :mHttpUrl(url), mAsioContext(io)
    {
        this->mCorComponent = App::Get().GetCoroutineComponent();
        this->mCorId = this->mCorComponent->GetCurrentCorId();
    }


    void HttpRequestTask::Run()
    {
        std::string host = "";
        std::string port = "";
        std::string path = "";
        if(!HttpUrlHelper::TryParse(this->mHttpUrl, host, port, path))
        {
            SayNoDebugError("parse " << this->mHttpUrl << " failure");
            return;
        }
        asio::error_code err;
        tcp::resolver resolver(mAsioContext);
        tcp::resolver::query query(host, port);
        tcp::resolver::iterator endpoint = resolver.resolve(query,err);
        if(err)
        {
            SayNoDebugError("{ " << this->mHttpUrl << " } " << err.message());
            return;
        }

        tcp::socket socket1(this->mAsioContext);
        asio::connect(socket1, endpoint, err);
        if(err)
        {
            SayNoDebugError(err.message());
            return;
        }
        SayNoDebugInfo("connect http host " << socket1.remote_endpoint().address() << ":" << port << "  successful");

        asio::streambuf  request;
        std::ostream requestStream(&request);

        requestStream << "GET " << path.c_str() << " HTTP/1.0\r\n";
        requestStream << "Host: " << host.c_str() << "\r\n";
        requestStream << "Accept: */*\r\n";
        requestStream << "Connection: close\r\n\r\n";

        size_t size = asio::write(socket1, request, err);
        if(err)
        {
            SayNoDebugError(err.message());
            return;
        }
        SayNoDebugLog("write message " << size);

        asio::streambuf response;
        size_t count = asio::read_until(socket1, response, "\r\n");
        SayNoDebugError("count = " << count);
        std::istream response_stream(&response);

        std::string message;
        while(std::getline(response_stream, message))
        {
            SayNoDebugWarning(message);
        }
    }

    void HttpRequestTask::RunFinish()
    {

    }
}

namespace Sentry
{
    bool HttpClientComponent::Awake()
    {
        this->mTaskComponent = this->GetComponent<TaskComponent>();
        this->mCorComponent = this->GetComponent<CoroutineComponent>();
        return true;
    }

    void HttpClientComponent::Start()
    {
        std::string json ;
        this->Get("http://timor.tech/api/holiday/year", json);
    }

    void HttpClientComponent::OnHttpContextUpdate(AsioContext & ctx)
    {

    }

    XCode HttpClientComponent::Get(const std::string &url, std::string &json, int timeout)
    {
      if (this->mCorComponent->IsInMainCoroutine())
      {
        return XCode::NoCoroutineContext;
      }
      HttpRequestTask httpTask(url, App::Get().GetHttpContext());
      this->mTaskComponent->StartTask("Http", &httpTask);
      this->mCorComponent->YieldReturn();


      return XCode::Successful;
    }
}