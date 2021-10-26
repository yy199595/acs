//
// Created by zmhy0073 on 2021/10/26.
//

#ifndef SENTRY_HTTPDOWNLOADREQUESTTASK_H
#define SENTRY_HTTPDOWNLOADREQUESTTASK_H
#include <fstream>
#include "HttpRequestTask.h"
namespace Sentry
{
    class HttpDownloadRequestTask : HttpRequestTask
    {
    public:
        using HttpRequestTask::HttpRequestTask;
    public:
        XCode Download(const std::string & path);
    protected:
        void GetSendData(asio::streambuf &streambuf) override;
        void OnReceiveBody(asio::streambuf &streambuf) override;

    private:
        std::string mFileName;
        std::fstream mFstream;
    };
}

#endif //SENTRY_HTTPDOWNLOADREQUESTTASK_H
