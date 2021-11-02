//
// Created by zmhy0073 on 2021/10/26.
//

#ifndef GameKeeper_HTTPREMOTESESSION_H
#define GameKeeper_HTTPREMOTESESSION_H
#include "HttpSessionBase.h"
namespace GameKeeper
{
    class HttpRemoteRequest;
    class HttpClientComponent;
    class HttpRemoteSession : public HttpSessionBase
    {
    public:
        explicit HttpRemoteSession(HttpClientComponent * socketHandler);
    public:
        SocketType GetSocketType() override { return SocketType::RemoteSocket;}
    protected:
        void OnSessionEnable() override;
        void OnSendHttpMessageAfter() override;
        bool WriterToBuffer(std::ostream &os) override;
        bool OnReceiveBody(asio::streambuf &buf, const asio::error_code &code) override;
        bool OnReceiveHeard(asio::streambuf &buf,size_t size, const asio::error_code &code) override;
    private:
        void DestoryHandler();
    private:
        std::string mMethod;
        HttpRemoteRequest * mHttpHandler;
        HttpClientComponent * mHttpComponent;
    };
}

#endif //GameKeeper_HTTPREMOTESESSION_H
