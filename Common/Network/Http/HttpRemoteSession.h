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
		void SetSocketProxy(SocketProxy * socketProxy) final;
	protected:
		HttpHandlerBase * GetHandler() final;
		bool OnReceiveHeard(asio::streambuf & buf) final;
	
		
    private:
		std::string mMethod;
        HttpClientComponent * mHttpComponent;
        HttpRequestHandler * mHttpHandler;
    };
}

#endif //GameKeeper_HTTPREMOTESESSION_H
