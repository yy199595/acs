//
// Created by zmhy0073 on 2021/10/29.
//

#ifndef GameKeeper_HTTPLOCALSESSION_H
#define GameKeeper_HTTPLOCALSESSION_H
#include"Http/Request/HttpRequest.h"
#include"Http/Response/HttpRespTask.h"
#include <Network/Http/HttpSessionBase.h>
namespace GameKeeper
{
    class HttpReqSession : public HttpSessionBase
    {
    public:
        explicit HttpReqSession(NetWorkThread & thread);

        ~HttpReqSession() final;

    public:

        SocketType GetSocketType() final { return SocketType::LocalSocket; }

    public:
        template<typename T1, typename T2>
        std::shared_ptr<T2> NewTask(std::shared_ptr<T1> request)
        {
            this->mHttpRequest = request;
            this->mHttpRespTask = std::make_shared<T2>();
            this->StartConnectHost(request->GetHost(), request->GetPort());
            return this->mHttpRespTask;
        }

    protected:

        void OnComplete(XCode code) final;

        void OnWriterAfter(XCode code) final;

        void WriterToBuffer(std::ostream &) final;

        bool OnReceiveHead(asio::streambuf &buf) final;

        bool OnReceiveBody(asio::streambuf &buf) final;

    private:
        void Resolver();

        void ConnectHandler(const asio::error_code &err);

        void StartConnectHost(const std::string &host, const std::string &port);

    private:
        std::string mHost;
        std::string mPort;
        NetWorkThread & mThread;
        asio::ip::tcp::resolver *mResolver;
        asio::ip::tcp::resolver::query *mQuery;
        std::shared_ptr<HttpRequest> mHttpRequest;
        std::shared_ptr<HttpRespTask> mHttpRespTask;

    };
}
#endif //GameKeeper_HTTPLOCALSESSION_H
