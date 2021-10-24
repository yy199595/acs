//
// Created by zmhy0073 on 2021/10/15.
//

#include "HttpRequestTask.h"
#include <Core/App.h>
#include "HttpClientSession.h"
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
#include<Util/TimeHelper.h>
namespace Sentry
{
    HttpRequestTask::HttpRequestTask(const std::string & url, AsioContext & io, std::string & res)
            :mHttpUrl(url), mAsioContext(io), mData(res)
    {     
        this->mHttpClient = nullptr;
        this->mCorComponent = App::Get().GetCoroutineComponent();
        this->mCorId = this->mCorComponent->GetCurrentCorId();
        this->mTaskComponent = App::Get().GetComponent<TaskPoolComponent>();
		this->mStartTime = TimeHelper::GetMilTimestamp();
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
		switch (err)
		{
		case EHttpError::HttpConnectError:
		case EHttpError::HttpReadError:
		case EHttpError::HttpWriteError:
			this->mCode = XCode::HttpNetWorkError;
			break;
		case EHttpError::HttpResolverError:
			this->mCode = XCode::HostResolverError;
			break;
		}
		int code = 0;
        std::string version;
        httpResponseStream >> version >> code;
        if (code != 200)
        {
			std::string err;
			while (std::getline(httpResponseStream, err))
			{
				SayNoDebugError(err);
			}
			this->mCode = XCode::HttpResponseError;
        }
		else
		{
			std::string content;
			this->mCode = XCode::Successful;
			while (std::getline(httpResponseStream, content))
			{
				if (content.size() == 1 && content == "\r")
				{
					break;
				}
			}
			std::istreambuf_iterator<char> eos;
			this->mData.append(std::istreambuf_iterator<char>(httpResponseStream), eos);
		}
       
        this->mTaskComponent->PushFinishTask(this->GetTaskId());
		SayNoDebugError("time = " << (TimeHelper::GetMilTimestamp() - this->mStartTime) / 1000.0f);
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
		SayNoDebugInfo("host = " << host << "  port = " << port);

        char buffer[1024] = {0};
        size_t size = sprintf(buffer, "GET %s HTTP/1.0\r\nHost: %s\r\nAccept: */*\r\nConnection: close\r\n\r\n", path.c_str(), host.c_str());

        //this->mHttpClient = new HttpClientSession(this->mAsioContext, host, port);
        IHttpResponseHandler * handler = new HttpResponseHandler<HttpRequestTask>(&HttpRequestTask::OnResponse, this);
        //this->mHttpClient->Request(std::make_shared<std::string>(buffer, size), handler);
        return false;
    }

    void HttpRequestTask::RunFinish()
    {
        this->mCorComponent->Resume(this->mCorId);
    }
}