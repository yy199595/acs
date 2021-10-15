//
// Created by zmhy0073 on 2021/10/15.
//

#ifndef SENTRY_HTTPCLIENTSESSION_H
#define SENTRY_HTTPCLIENTSESSION_H
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
    class IHttpReponseHandler
    {
    public:
        virtual void Run(EHttpError code, SharedMessage response) = 0;
    };

    template<typename T>
    class HttpResponseHandler : public IHttpReponseHandler
    {
    public:
        typedef void (T::*HandlerFunc)(EHttpError, SharedMessage);
        HttpResponseHandler(HandlerFunc && f, T * o):_o(o), _f(std::forward<HandlerFunc>(f)) { }
    public:
        void Run(EHttpError code, SharedMessage response) override
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

    class HttpClientSession
    {
    public:
        HttpClientSession(AsioContext &io, const std::string &host, const std::string &port = "80");

        ~HttpClientSession();

    public:
        void Request(SharedMessage message, IHttpReponseHandler *handler);

    private:
        void ConnectHandler(const asio::error_code &err);

        void ResolverHandler(const asio::error_code &err, tcp::resolver::iterator iterator);

        void WriteHandler(const asio::error_code &err, const size_t size);

        void ReadHeadHandler(const asio::error_code &err, const size_t size);

        void ReadReadHandler(const asio::error_code &err, const size_t size);

        void ReadBodyHandler(const asio::error_code &err, const size_t size);

    private:
        AsioContext &mHttpContext;
        SharedTcpSocket mHttpSocket;
        SharedMessage mRequestMessage;
        IHttpReponseHandler *mResponseHandler;
    private:
        tcp::resolver mResolver;
        tcp::resolver::query mQuery;
    private:
        asio::streambuf response;
        std::istream response_stream;
    };
}
#endif //SENTRY_HTTPCLIENTSESSION_H
