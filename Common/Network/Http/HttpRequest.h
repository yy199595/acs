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
        virtual void GetRquestParame(asio::streambuf &strem) = 0;

        bool Connect(const std::string &host, unsigned short port);

        bool OnReceive(asio::streambuf &strem, const asio::error_code &err);

        virtual void OnReceiveDone(bool hasError) = 0;
    protected:

        virtual void OnReceiveBody(asio::streambuf &strem) = 0;

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
    private:
        bool mIsEnd;
        int mHttpCode;
        int mReadCount;
        std::string mHeard;
        std::string mError;
        std::string mVersion;
    };
}

namespace Sentry
{
	
	class HttpGetRequest : public HttpRequest
	{
	public:
		HttpGetRequest(ISocketHandler * handler);
	public:
		XCode Get(const std::string & url, std::string & response);
    protected:
        void OnReceiveDone(bool hasError) override;
        void OnReceiveBody(asio::streambuf &strem) override;
        void GetRquestParame(asio::streambuf & strem) override;
    private:
		XCode mCode;
		std::string mPath;
		std::string mName;
        std::string mData;
		unsigned int mCoroutineId;
		MainTaskScheduler & mMainTaskPool;
		CoroutineComponent * mCorComponent;
	};
}

namespace Sentry
{
	class HttpPostRequest : public HttpRequest
	{
	public:

	};
}