//
// Created by zmhy0073 on 2021/11/4.
//

#ifndef GAMEKEEPER_HTTPREMOTEPOSTREQUESTHANDLER_H
#define GAMEKEEPER_HTTPREMOTEPOSTREQUESTHANDLER_H
#include "HttpRemoteRequestHandler.h"
#include <Network/Http/Content/HttpReadContent.h>
namespace GameKeeper
{
    class HttpRemotePostRequestHandler : public HttpRemoteRequestHandler
    {
    public:
        explicit HttpRemotePostRequestHandler(HttpClientComponent *component, HttpRemoteSession *session);
        ~HttpRemotePostRequestHandler() override = default;
    public:
        XCode GetContent(HttpReadContent & content);
        void OnReceiveBodyAfter(XCode code) override;
    protected:
        unsigned int mCorId;
        HttpReadContent * mContent;
        void OnReceiveBody(asio::streambuf &buf) override;
    };
}
#endif //GAMEKEEPER_HTTPREMOTEPOSTREQUESTHANDLER_H
