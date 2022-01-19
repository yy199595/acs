//
// Created by zmhy0073 on 2022/1/19.
//

#ifndef GAMEKEEPER_HTTPASYNCREQUEST_H
#define GAMEKEEPER_HTTPASYNCREQUEST_H
#include<string>
#include<asio.hpp>
#include<Http/Http.h>
#include<unordered_map>
namespace GameKeeper
{
    class IHttpStream
    {
    public:
        virtual asio::streambuf & GetStream() = 0;
    };
}

namespace GameKeeper
{
    class HttpAsyncRequest : public IHttpStream
    {
    public:
        bool Get(const std::string & url);
        bool Post(const std::string & url, const std::string & content);

    public:
        const std::string & GetHost() { return this->mHost;}
        const std::string & GetPort() { return this->mPort;}
        asio::streambuf & GetStream()  final { return this->mSendStream;}
    private:
        bool ParseUrl(const std::string & url);
    private:
        std::string mUrl;
        std::string mHost;
        std::string mPort;
        std::string mPath;
        asio::streambuf mSendStream;
    };
}

namespace GameKeeper
{
    enum class HttpDecodeState
    {
        FirstLine,
        HeadLine,
        Content,
        Finish
    };
    class IHttpContent
    {
    public:
        virtual const std::string & GetContent() = 0;
        virtual HttpStatus OnReceiveData(asio::streambuf & streamBuffer) = 0;
    };
}

namespace GameKeeper
{
    class HttpAsyncResponse : public IHttpContent
    {
    public:
        HttpAsyncResponse();
    public:
        HttpStatus OnReceiveData(asio::streambuf &streamBuffer) final;
        HttpStatus GetHttpCode() { return (HttpStatus)this->mHttpCode;}
        const std::string & GetContent() final { return this->mContent;}
    private:
        int mHttpCode;
        std::string mContent;
        std::string mVersion;
        std::string mHttpError;
        HttpDecodeState mState;
        size_t mContentLength;
        std::unordered_map<std::string, std::string> mHeadMap;
    };
}

namespace GameKeeper
{
    class HttpHandlerRequest : public IHttpContent
    {
    public:
        HttpHandlerRequest();
    public:
        HttpStatus OnReceiveData(asio::streambuf &streamBuffer) final;
        const std::string & GetMethod() { return this->mMethod; }
        const std::string & GetContent() final { return this->mContent; }
    private:
        std::string mUrl;
        std::string mMethod;
        std::string mContent;
        std::string mVersion;
        HttpDecodeState mState;
        size_t mContentLength;
        std::unordered_map<std::string, std::string> mHeadMap;
    };
}
#endif //GAMEKEEPER_HTTPASYNCREQUEST_H
