//
// Created by zmhy0073 on 2022/1/19.
//

#include"HttpHandlerClient.h"

namespace GameKeeper
{
    HttpHandlerClient::HttpHandlerClient(std::shared_ptr<SocketProxy> socketProxy)
    {
        this->mSocket = socketProxy;
    }

    std::shared_ptr<HttpHandlerRequest> HttpHandlerClient::ReadHandlerContent()
    {
        NetWorkThread & netWorkThread = this->mSocket->GetThread();
        std::shared_ptr<TaskSource<bool>> taskSource(new TaskSource<bool>());
        std::shared_ptr<HttpHandlerRequest> handlerRequest(new HttpHandlerRequest());
        netWorkThread.Invoke(&HttpHandlerClient::ReadHttpData, this, taskSource, handlerRequest);
        if(!taskSource->Await())
        {
            return nullptr;
        }
        return handlerRequest;
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
                    switch (handlerRequest->OnReceiveData(this->mStreamBuffer))
                    {
                        case HttpStatus::CONTINUE:
                            this->ReadHttpData(taskSource, handlerRequest);
                            break;
                        case HttpStatus::OK:
                            taskSource->SetResult(true);
                            break;
                        default:
                            break;
                    }
                });
    }
}