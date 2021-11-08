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
		HttpRequestHandler * GetReuqestHandler() { return this->mHttpHandler; }
    public:
        void Clear() final;
		HttpHandlerBase * GetHandler() final;
        size_t ReadFromStream(char * buffer, size_t count);
	protected:
		
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
        HttpRequestHandler * mHttpHandler;
        HttpClientComponent * mHttpComponent;
        std::unordered_map<std::string, HttpRequestHandler *> mHandlerMap;
    };
}

#endif //GameKeeper_HTTPREMOTESESSION_H
