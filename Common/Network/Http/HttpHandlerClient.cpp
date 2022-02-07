//
// Created by zmhy0073 on 2022/1/19.
//

#include"HttpHandlerClient.h"

namespace Sentry
{
    HttpHandlerClient::HttpHandlerClient(std::shared_ptr<SocketProxy> socketProxy)
    {
        this->mSocket = socketProxy;
    }

    std::shared_ptr<HttpHandlerRequest> HttpHandlerClient::ReadHandlerContent()
    {
        std::shared_ptr<TaskSource<bool>> taskSource(new TaskSource<bool>());
        std::shared_ptr<HttpHandlerRequest> handlerRequest(new HttpHandlerRequest());
#ifdef ONLY_MAIN_THREAD
        this->ReadHttpData(taskSource, handlerRequest);
#else
        IAsioThread & netWorkThread = this->mSocket->GetThread();
        netWorkThread.Invoke(&HttpHandlerClient::ReadHttpData, this, taskSource, handlerRequest);
#endif
        return taskSource->Await() ? handlerRequest : nullptr;
    }

    bool HttpHandlerClient::Response(HttpStatus code, RapidJsonWriter &jsonWriter)
    {
        std::shared_ptr<HttpHandlerResponse> response(new HttpHandlerResponse(code));
        response->AddValue(jsonWriter);
        return this->Response(response);
    }

    bool HttpHandlerClient::Response(std::shared_ptr<HttpHandlerResponse> response)
    {
        std::shared_ptr<TaskSource<bool>> taskSource(new TaskSource<bool>());
#ifdef ONLY_MAIN_THREAD
        this->ResponseData(taskSource, response);
#else
        IAsioThread & netWorkThread = this->mSocket->GetThread();
        netWorkThread.Invoke(&HttpHandlerClient::ResponseData, this, taskSource, response);
#endif
        return taskSource->Await();
    }

    void HttpHandlerClient::ResponseData(std::shared_ptr<TaskSource<bool>> taskSource,
                                         std::shared_ptr<HttpHandlerResponse> response)
    {
        asio::streambuf & streambuf = response->GetStream();
        AsioTcpSocket &tcpSocket = this->mSocket->GetSocket();
        std::shared_ptr<HttpHandlerClient> self = this->shared_from_this();
        asio::async_write(tcpSocket, streambuf, [this, self, taskSource, response]
                (const asio::error_code & code, size_t size)
        {
            if(code)
            {
                STD_ERROR_LOG(code.message());
                taskSource->SetResult(false);
                return;
            }
            taskSource->SetResult(true);
        });
    }

    void HttpHandlerClient::ReadHttpData(std::shared_ptr<TaskSource<bool>> taskSource,
                                         std::shared_ptr<HttpHandlerRequest> handlerRequest)
    {
        AsioTcpSocket &tcpSocket = this->mSocket->GetSocket();
        asio::async_read(tcpSocket, this->mStreamBuffer,asio::transfer_at_least(1),
                         [this, taskSource, handlerRequest] (const asio::error_code &code, size_t size)
                {
                    if (code)
                    {
                        STD_ERROR_LOG(code.message());
                        taskSource->SetResult(false);
                        return;
                    }
                    HttpStatus httpCode = handlerRequest->OnReceiveData(this->mStreamBuffer);
                    switch (httpCode)
                    {
                        case HttpStatus::CONTINUE:
                            this->ReadHttpData(taskSource, handlerRequest);
                            break;
                        case HttpStatus::OK:
                            taskSource->SetResult(true);
                            break;
                        default:
                            taskSource->SetResult(false);
                            break;
                    }
                });
    }
}