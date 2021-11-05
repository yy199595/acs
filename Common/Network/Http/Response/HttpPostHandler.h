//
// Created by zmhy0073 on 2021/11/4.
//

#ifndef GAMEKEEPER_HTTPPOSTHANDLER_H
#define GAMEKEEPER_HTTPPOSTHANDLER_H
#include "HttpRequestHandler.h"
#include <Network/Http/Content/HttpReadContent.h>
namespace GameKeeper
{
    class HttpPostHandler : public HttpRequestHandler
    {
    public:
        explicit HttpPostHandler(HttpClientComponent *component, HttpRemoteSession *session);
        ~HttpPostHandler() override = default;
    public:
        const std::string & GetPath() override;
        void OnReceiveBodyAfter(XCode code) override;
        void OnReceiveHeardAfter(XCode code) override;
        HttpMethodType GetType() final { return HttpMethodType::POST; }
        bool OnReceiveHeard(asio::streambuf & buf, size_t size) override;

        const std::string & GetParameter() override { return this->mContent->GetContent(); }
    protected:
        HttpReadContent * mContent;
        void OnReceiveBody(asio::streambuf &buf) override;
    };
}
#endif //GAMEKEEPER_HTTPPOSTHANDLER_H
