//
// Created by zmhy0073 on 2021/10/26.
//

#ifndef SENTRY_HTTPGETREQUESTTASK_H
#define SENTRY_HTTPGETREQUESTTASK_H
#include "HttpRequestTask.h"
namespace Sentry
{
    class HttpGetRequestTask : public HttpRequestTask
    {
    public:
        HttpGetRequestTask(const std::string & url);

    public:
        XCode Get(std::string & response);
    protected:
        void GetSendData(asio::streambuf &streambuf) override;
        void OnReceiveBody(asio::streambuf &streambuf) override;

    private:
        std::string * mResponse;
    };
}

#endif //SENTRY_HTTPGETREQUESTTASK_H
