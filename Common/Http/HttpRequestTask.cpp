//
// Created by zmhy0073 on 2021/10/15.
//

#include "HttpRequestTask.h"
#include <Core/App.h>
#include <Http/HttpClientSession.h>
#include <Scene/TaskPoolComponent.h>
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
    HttpRequestTask::HttpRequestTask(const std::string & url, AsioContext & io, std::string & res)
            :mHttpUrl(url), mAsioContext(io), mData(res)
    {
        this->mCode = 0;
        this->mHttpClient = nullptr;
        this->mCorComponent = App::Get().GetCoroutineComponent();
        this->mCorId = this->mCorComponent->GetCurrentCorId();
        this->mTaskComponent = App::Get().GetComponent<TaskPoolComponent>();
    }

    HttpRequestTask::~HttpRequestTask() noexcept
    {
        if(this->mHttpClient != nullptr)
        {
            delete this->mHttpClient;
        }
    }

    void HttpRequestTask::OnResponse(EHttpError err, HttpResponseStream & httpResponseStream)
    {
        if(err != EHttpError::HttpSuccessful)
        {
            this->mCode = 1;
            return;
        }
        std::string version;
        httpResponseStream >> version >> this->mCode;
        if (this->mCode != 200)
        {
            return;
        }
        std::string content;
        while (std::getline(httpResponseStream, content))
        {
            if (content.size() == 1 && content == "\r")
            {
                break;
            }
        }
        std::istreambuf_iterator<char> eos;
        this->mData.append(std::istreambuf_iterator<char>(httpResponseStream), eos);
        this->mTaskComponent->PushFinishTask(this->GetTaskId());
    }


    bool HttpRequestTask::Run()
    {
        std::string host = "";
        std::string port = "";
        std::string path = "";
        if (!HttpUrlHelper::TryParse(this->mHttpUrl, host, port, path))
        {
            SayNoDebugError("parse " << this->mHttpUrl << " failure");
            return true;
        }

        char buffer[1024] = {0};
        size_t size = sprintf(buffer, "GET %s HTTP/1.0\r\nHost: %s\r\nAccept: */*\r\nConnection: close\r\n\r\n", path.c_str(), host.c_str());

        this->mHttpClient = new HttpClientSession(this->mAsioContext, host, port);
        IHttpReponseHandler * handler = new HttpResponseHandler<HttpRequestTask>(&HttpRequestTask::OnResponse, this);
        this->mHttpClient->Request(std::make_shared<std::string>(buffer, size), handler);
        return false;
    }

    void HttpRequestTask::RunFinish()
    {
        this->mCorComponent->Resume(this->mCorId);
    }
}