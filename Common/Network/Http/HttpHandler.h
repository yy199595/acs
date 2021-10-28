//
// Created by zmhy0073 on 2021/10/26.
//

#ifndef SENTRY_HTTPHANDLER_H
#define SENTRY_HTTPHANDLER_H

#include <unordered_map>
#include <Define/CommonTypeDef.h>
namespace Sentry
{
    class HttpRemoteSession;
    class HttpHandler
    {
    public:
        HttpHandler(HttpRemoteSession * session);
        virtual ~HttpHandler();
    public:
        void ResponseData(int code, std::string * data);
        bool GetHeardData(const std::string & key, std::string & value);
        bool OnReceive(asio::streambuf & streambuf, const asio::error_code & err);
        void OnReceiveHeard(const std::string & path, const std::string & version, const std::string & heard);
    protected:
        virtual void OnReceiveDone() = 0;
        virtual void OnReceiveBody(asio::streambuf & streambuf) = 0;
    protected:
        std::string mPath;
        std::string mVersion;
        HttpRemoteSession * mHttpSession;
    private:
        asio::streambuf mResponseBuf;
        std::unordered_map<std::string, std::string> mHeardMap;
    };
}

#endif //SENTRY_HTTPHANDLER_H
