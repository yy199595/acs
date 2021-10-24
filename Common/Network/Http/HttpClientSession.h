//
// Created by zmhy0073 on 2021/10/15.
//

#ifndef SENTRY_HTTPCLIENTSESSION_H
#define SENTRY_HTTPCLIENTSESSION_H
#include <Network/SessionBase.h>
#include <Define/CommonTypeDef.h>
using namespace asio::ip;

namespace Sentry
{
    enum EHttpError
    {
        HttpSuccessful,
        HttpResolverError,
        HttpConnectError,
        HttpWriteError,
        HttpReadError,
        HttpResponseError,
    };
}

namespace Sentry
{
    typedef std::istream HttpResponseStream;
    class IHttpResponseHandler
    {
    public:
        virtual void Run(EHttpError code, std::istream & response) = 0;
    };

    template<typename T>
    class HttpResponseHandler : public IHttpResponseHandler
    {
    public:
        typedef void (T::*HandlerFunc)(EHttpError, HttpResponseStream &);
        HttpResponseHandler(HandlerFunc && f, T * o):_o(o), _f(std::forward<HandlerFunc>(f)) { }
    public:
        void Run(EHttpError code, HttpResponseStream & response) override
        {
            (_o->*_f)(code, response);
        }

    private:
        T* _o;
        HandlerFunc _f;
    };
}

namespace Sentry
{
    class ISocketHandler;
    class HttpRequestTask;

    class HttpClientSession : public SessionBase
    {
    public:
        HttpClientSession(ISocketHandler * handler);

        ~HttpClientSession();

    private:
        void ConnectHandler(const asio::error_code &err);

        void WriteHandler(const asio::error_code &err, const size_t size);

        void ReadHeadHandler(const asio::error_code &err, const size_t size);

        void ReadBodyHandler(const asio::error_code &err, const size_t size);

    protected:
        void OnSessionEnable() override;

        void OnConnect(const asio::error_code &err) override;
    private:
        asio::streambuf mResponseBuf;
        HttpResponseStream mResponseStream;
    };
}
#endif //SENTRY_HTTPCLIENTSESSION_H
