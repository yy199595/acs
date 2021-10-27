//
// Created by zmhy0073 on 2021/10/27.
//

#ifndef SENTRY_HTTPPOSTREQUESTTASK_H
#define SENTRY_HTTPPOSTREQUESTTASK_H
#include "HttpRequestTask.h"

namespace Sentry
{
    class HttpPostRequestTask : public HttpRequestTask
    {
    public:
        using HttpRequestTask::HttpRequestTask;
    public:
        XCode Post(const std::string & data, std::string & response);
    protected:
        void GetSendData(asio::streambuf &streambuf) override;
        void OnReceiveBody(asio::streambuf &streambuf) override;
    private:
        const std::string * mPostData;
        std::string * mResponseData;
    };
}
#endif //SENTRY_HTTPPOSTREQUESTTASK_H
