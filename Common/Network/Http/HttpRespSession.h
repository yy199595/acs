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
    class HttpRespSession : public HttpSessionBase
    {
    public:
        explicit HttpRespSession(HttpClientComponent * socketHandler);
        ~HttpRespSession() final;
    public:
        void Start(SocketProxy * socketProxy);
		SocketType GetSocketType() final { return SocketType::RemoteSocket; }
		HttpRequestHandler * GetReuqestHandler() { return this->mHttpHandler; }
	protected:
        void OnComplete(XCode code) final;
        void OnWriterAfter(XCode code) final;
        void WriterToBuffer(std::ostream & os) final;
        bool OnReceiveHead(asio::streambuf &buf) final;
        bool OnReceiveBody(asio::streambuf &buf) final;
    private:
        size_t mWriterCount;
        unsigned int mCorId;
    private:
		std::string mMethod;
        HttpRequestHandler * mHttpHandler;
        HttpClientComponent * mHttpComponent;
        std::unordered_map<std::string, HttpRequestHandler *> mHandlerMap;
    };
}

#endif //GameKeeper_HTTPREMOTESESSION_H
