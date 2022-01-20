//
// Created by zmhy0073 on 2022/1/19.
//

#ifndef GAMEKEEPER_HTTPHANDLERCLIENT_H
#define GAMEKEEPER_HTTPHANDLERCLIENT_H
#include"HttpAsyncRequest.h"
#include"Async/TaskSource.h"
#include"Network/SocketProxy.h"
namespace GameKeeper
{
class HttpHandlerClient : public std::enable_shared_from_this<HttpHandlerClient>
    {
    public:
        HttpHandlerClient(std::shared_ptr<SocketProxy> socketProxy);
        ~HttpHandlerClient() {LOG_ERROR("delete http handler client => ", this->mSocket->GetAddress());}
    public:
        bool SendResponse(HttpStatus status);
        bool SendResponse(HttpStatus status, const std::string & content);
        std::shared_ptr<HttpHandlerRequest> ReadHandlerContent();
    private:
        void ResponseData(std::shared_ptr<TaskSource<bool>> taskSource);
        void ReadHttpData(std::shared_ptr<TaskSource<bool>> taskSource, std::shared_ptr<HttpHandlerRequest> handlerRequest);
    private:
        asio::streambuf mStreamBuffer;
        HttpHandlerResponse mResponseData;
        std::shared_ptr<SocketProxy> mSocket;
    };
}
#endif //GAMEKEEPER_HTTPHANDLERCLIENT_H
