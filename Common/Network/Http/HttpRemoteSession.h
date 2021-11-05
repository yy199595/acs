//
// Created by zmhy0073 on 2021/10/26.
//

#ifndef GameKeeper_HTTPREMOTESESSION_H
#define GameKeeper_HTTPREMOTESESSION_H
#include "HttpSessionBase.h"
namespace GameKeeper
{

    class HttpClientComponent;
    class HttpRequestHandler;
    class HttpRemoteSession : public HttpSessionBase
    {
    public:
        explicit HttpRemoteSession(HttpClientComponent * socketHandler);
    public:
		void SetSocketProxy(SocketProxy * socketProxy) override;
    protected:        
        void OnWriteAfter(XCode code) override;
        void OnReceiveBodyAfter(XCode code) override;
        void OnReceiveHeardAfter(XCode code) override;
        bool WriterToBuffer(std::ostream &os) override;
        void OnReceiveBody(asio::streambuf &buf) override;
        bool OnReceiveHeard(asio::streambuf &buf,size_t size) override;
    private:
        HttpClientComponent * mHttpComponent;
        HttpRequestHandler * mHttpHandler;
    };
}

#endif //GameKeeper_HTTPREMOTESESSION_H
