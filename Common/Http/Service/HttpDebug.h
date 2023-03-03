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
    class HttpDebug: public LocalHttpService
    {
    public:
        HttpDebug() = default;
    public:
        bool OnStartService(HttpServiceRegister &serviceRegister) final;
    private:
        int Call(const Http::Request& request, Http::Response& response);
        int Invoke(std::shared_ptr<Rpc::Packet> data, std::shared_ptr<Json::Writer> document);

    };
}

#endif //APP_HTTPRPCSERVICE_H
