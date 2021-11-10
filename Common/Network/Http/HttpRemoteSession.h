//
// Created by zmhy0073 on 2021/10/26.
//

#ifndef GameKeeper_HTTPREMOTESESSION_H
#define GameKeeper_HTTPREMOTESESSION_H
#include "HttpSessionBase.h"
namespace GameKeeper
{
    class HttpComponent;
    class HttpRequestHandler;
    class HttpRemoteSession : public HttpSessionBase
    {
    public:
        explicit HttpRemoteSession(HttpComponent * socketHandler);
        ~HttpRemoteSession() final;
    public:
        void Start(SocketProxy * socketProxy);
		SocketType GetSocketType() final { return SocketType::RemoteSocket; }
		HttpRequestHandler * GetReuqestHandler() { return this->mHttpHandler; }
    public:
        void Clear() final;
	protected:
        void OnWriterAfter(XCode code) final;
        void OnReceiveHeadAfter(XCode code) final;
        bool WriterToBuffer(std::ostream &) final;
        void OnReceiveHeard(asio::streambuf & buf) final;

    private:
        void StartReceiveBody();
        void SetCode(XCode code);
        void ReadBodyCallback(const asio::error_code & err, size_t size);
    private:
        size_t mWriterCount;
        unsigned int mCorId;
    private:
		std::string mMethod;
        HttpRequestHandler * mHttpHandler;
        HttpComponent * mHttpComponent;
        std::unordered_map<std::string, HttpRequestHandler *> mHandlerMap;
    };
}

#endif //GameKeeper_HTTPREMOTESESSION_H
