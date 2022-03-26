//
// Created by zmhy0073 on 2022/1/19.
//

#ifndef GAMEKEEPER_HTTPHANDLERCLIENT_H
#define GAMEKEEPER_HTTPHANDLERCLIENT_H
#include"HttpAsyncRequest.h"
#include"Async/TaskSource.h"
#include"Network/SocketProxy.h"
#include"Json/JsonWriter.h"
namespace Sentry
{
class HttpHandlerClient : public std::enable_shared_from_this<HttpHandlerClient>
    {
    public:
        HttpHandlerClient(std::shared_ptr<SocketProxy> socketProxy);
    public:
        std::shared_ptr<HttpHandlerRequest> ReadHandlerContent();
        bool Response(std::shared_ptr<HttpHandlerResponse> response);
        bool Response(HttpStatus code, Json::Writer & jsonWriter);

    private:
        void ResponseData(std::shared_ptr<TaskSource<bool>> taskSource, std::shared_ptr<HttpHandlerResponse> response);
        void ReadHttpData(std::shared_ptr<TaskSource<bool>> taskSource, std::shared_ptr<HttpHandlerRequest> handlerRequest);
    private:
        asio::streambuf mStreamBuffer;
        std::shared_ptr<SocketProxy> mSocket;
    };
}
#endif //GAMEKEEPER_HTTPHANDLERCLIENT_H
