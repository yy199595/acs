//
// Created by zmhy0073 on 2021/10/26.
//

#ifndef GameKeeper_HTTPREMOTESESSION_H
#define GameKeeper_HTTPREMOTESESSION_H
#include "HttpSessionBase.h"
namespace GameKeeper
{
    class HttpNullHandler;
    class HttpClientComponent;
    class HttpRequestHandler;
    class HttpRemoteSession : public HttpSessionBase
    {
    public:
        explicit HttpRemoteSession(HttpClientComponent * socketHandler);
        ~HttpRemoteSession() final;
    public:
        void Start(SocketProxy * socketProxy);
		SocketType GetSocketType() final { return SocketType::LocalSocket; }

    public:
        void Clear() final;
        size_t ReadFromStream(char * buffer, size_t count);
	protected:
		HttpHandlerBase * GetHandler() final;
        void OnWriterAfter(XCode code) final;
        void OnReceiveHeardAfter(XCode code) final;
        bool OnReceiveHeard(asio::streambuf & buf) final;

    private:
        void StartReceiveBody();
        void ReadBodyCallback(const asio::error_code & err, size_t size);

    private:
        size_t mReadSize;
        unsigned int mCorId;
    private:
		std::string mMethod;
        SocketProxy * mSocketProxy;
        HttpRequestHandler * mHttpHandler;
        HttpClientComponent * mHttpComponent;
        std::unordered_map<std::string, HttpRequestHandler *> mHandlerMap;
    };
}

#endif //GameKeeper_HTTPREMOTESESSION_H
