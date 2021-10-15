//
// Created by zmhy0073 on 2021/10/15.
//

#include "HttpRequestTask.h"
#include <Core/App.h>
#include <Http/HttpClientSession.h>
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
    HttpRequestTask::HttpRequestTask(const std::string & url, AsioContext & io)
            :mHttpUrl(url), mAsioContext(io)
    {
        this->mCorComponent = App::Get().GetCoroutineComponent();
        this->mCorId = this->mCorComponent->GetCurrentCorId();
    }

    void HttpRequestTask::OnResponse(EHttpError err, SharedMessage message)
    {

    }


    void HttpRequestTask::Run()
    {
        std::string host = "";
        std::string port = "";
        std::string path = "";
        const long long t1 = TimeHelper::GetMilTimestamp();
        if (!HttpUrlHelper::TryParse(this->mHttpUrl, host, port, path))
        {
            SayNoDebugError("parse " << this->mHttpUrl << " failure");
            return;
        }
        std::shared_ptr<std::string> requet = std::make_shared<std::string>();

        std::stringstream requestStream;

        requestStream << "GET " << path << " HTTP/1.0\r\n";
        requestStream << "Host: " << host << "\r\n";
        requestStream << "Accept: */*\r\n";
        requestStream << "Connection: close\r\n\r\n";

        char buffer[1024] = {0};
        size_t size = sprintf(buffer, "GET %s HTTP/1.0\r\nHost: %s\r\nAccept: */*\r\nConnection: close\r\n\r\n", path.c_str(), host.c_str());

        HttpClientSession * httpClient = new HttpClientSession(this->mAsioContext, host, port);
        IHttpReponseHandler * handler = new HttpResponseHandler<HttpRequestTask>(&HttpRequestTask::OnResponse, this);
        httpClient->Request(std::make_shared<std::string>(buffer, size), handler);

//        asio::error_code err;
//        tcp::resolver resolver(mAsioContext);
//        tcp::resolver::query query(host, port);
//        tcp::resolver::iterator endpoint = resolver.resolve(query, err);
//        if (err)
//        {
//            SayNoDebugError("{ " << this->mHttpUrl << " } " << err.message());
//            return;
//        }
//
//        tcp::socket socket1(this->mAsioContext);
//        asio::connect(socket1, endpoint, err);
//        if (err)
//        {
//            SayNoDebugError(err.message());
//            return;
//        }
//        SayNoDebugInfo("connect http host " << socket1.remote_endpoint().address() << ":" << port << "  successful");
//
//        asio::streambuf request;
//        std::ostream requestStream(&request);
//
//        requestStream << "GET " << path << " HTTP/1.0\r\n";
//        requestStream << "Host: " << host << "\r\n";
//        requestStream << "Accept: */*\r\n";
//        requestStream << "Connection: close\r\n\r\n";
//
//        size_t size = asio::write(socket1, request, err);
//
//        if (err)
//        {
//            SayNoDebugError(err.message());
//            return;
//        }
//        SayNoDebugLog("write message " << size);
//
//        asio::streambuf response;
//        asio::read_until(socket1, response, "\r\n");
//
//
//        std::istream response_stream(&response);
//        while (asio::read(socket1, response, asio::transfer_at_least(1), err))
//        {
//
//        }
//
//        std::string responseMessage;
//        while (std::getline(response_stream, responseMessage))
//        {
//            //SayNoDebugError(responseMessage);
//        }

        //SayNoDebugError(responseMessage);
        SayNoDebugFatal("时间 = " << ((TimeHelper::GetMilTimestamp() - t1) / 1000.0f) << "s");
    }

    void HttpRequestTask::RunFinish()
    {
        this->mCorComponent->Resume(this->mCorId);
    }
}