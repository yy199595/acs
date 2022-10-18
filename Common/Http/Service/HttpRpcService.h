//
// Created by zmhy0073 on 2022/8/11.
//

#ifndef APP_HTTPRPCSERVICE_H
#define APP_HTTPRPCSERVICE_H
#include"Client/Http.h"
#include"Client/Message.h"
#include"LocalHttpService.h"

namespace Sentry
{
    class HttpHandlerClient;
    class HttpRpcService: public LocalHttpService
    {
    public:
        HttpRpcService() = default;
    public:
        bool OnStartService(HttpServiceRegister &serviceRegister) final;
    private:
        XCode Call(const HttpHandlerRequest& request, HttpHandlerResponse& response);
        XCode Invoke(std::shared_ptr<Rpc::Packet> data, std::shared_ptr<Json::Document> document);

    };
}

#endif //APP_HTTPRPCSERVICE_H
