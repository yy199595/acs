//
// Created by zmhy0073 on 2021/10/25.
//

#ifndef SENTRY_HTTPDOWNLOADREQUEST_H
#define SENTRY_HTTPDOWNLOADREQUEST_H
#include <fstream>
#include "HttpRequest.h"
namespace Sentry
{
    class ISocketHandler;
    class HttpDownLoadRequest : HttpRequest
    {
    public:
        HttpDownLoadRequest(ISocketHandler * handler);
        ~HttpDownLoadRequest();
    public:
        XCode DownLoad(const std::string & url, const std::string & path);
    protected:
        void OnReceiveDone(bool hasError) override;
        void OnReceiveBody(asio::streambuf &strem) override;
        void GetRquestParame(asio::streambuf &strem) override;
        void OnReceiveHeard(const std::string &heard) override;
    private:
        XCode mCode;
        size_t mMaxSize;
        size_t mCurSize;
        char buffer[512];
        std::string mPath;
        unsigned int mCoreId;
        std::string mSavePath;
        std::string mFileName;
        std::fstream * mFileStream;
        class CoroutineComponent * mCorComponent;
    };
}

#endif //SENTRY_HTTPDOWNLOADREQUEST_H

