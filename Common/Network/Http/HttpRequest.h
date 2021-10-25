#pragma once
#include<string>
#include<XCode/XCode.h>
#include<Define/CommonTypeDef.h>
namespace Sentry
{
    class ISocketHandler;

    class MainTaskScheduler;

    class CoroutineComponent;

    class HttpLocalSession;

    class HttpRequest
    {
    public:
        HttpRequest(ISocketHandler *handler, const std::string &name);

        virtual ~HttpRequest();

    public:
        bool ConnectRemote();

        virtual void GetRquestParame(asio::streambuf &strem) = 0;

        bool OnReceive(asio::streambuf &strem, const asio::error_code &err);

        virtual void OnReceiveDone(bool hasError) = 0;
    protected:

        virtual void OnReceiveBody(asio::streambuf &strem) = 0;
        virtual void OnReceiveHeard(const std::string & heard) { };

    public:
        unsigned short GetPort()
        { this->mPort; }

        int GetHttpCode()
        { return this->mHttpCode; }

        const std::string &GetName()
        { return this->mName; }

        const std::string &GetHost()
        { return this->mHost; }

    protected:
        std::string mHost;
        unsigned short mPort;
        const std::string mName;
        HttpLocalSession *mHttpSession;
        MainTaskScheduler & mMainTaskPool;
    private:
        bool mIsEnd;
        int mHttpCode;
        int mReadCount;
        std::string mError;
        std::string mVersion;
    };
}