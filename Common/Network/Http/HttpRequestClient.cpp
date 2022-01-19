//
// Created by yjz on 2022/1/19.
//
#include"HttpRequestClient.h"
#include"Http.h"
#include<regex>
namespace GameKeeper
{
    HttpRequestClient::HttpRequestClient(std::shared_ptr<SocketProxy> socketProxy)
    {
        this->mSocket = socketProxy;
    }

    std::shared_ptr<HttpAsyncResponse> HttpRequestClient::Get(const std::string &url)
    {
        auto httpRequest = std::make_shared<HttpAsyncRequest>();
        if (!httpRequest->Get(std::move(url)))
        {
            return nullptr;
        }
        return this->Request(httpRequest);
    }

    std::shared_ptr<HttpAsyncResponse> HttpRequestClient::Request(std::shared_ptr<HttpAsyncRequest> httpRequest)
    {
        const std::string & host = httpRequest->GetHost();
        const std::string & port = httpRequest->GetPort();
        NetWorkThread & netWorkThread = this->mSocket->GetThread();
        std::shared_ptr<TaskSource<XCode>> taskSource(new TaskSource<XCode>);
        netWorkThread.Invoke(&HttpRequestClient::ConnectHost, this, host, port, taskSource);
        if(taskSource->Await() != XCode::Successful)
        {
            LOG_ERROR("connect http host ", host, ':', port, " failure");
            return nullptr;
        }
        LOG_INFO("connect http host ", host, ':', port, " successful");
        std::shared_ptr<TaskSource<bool>> sendTaskSource(new TaskSource<bool>);
        netWorkThread.Invoke(&HttpRequestClient::SendByStream, this, httpRequest, sendTaskSource);
        if(!sendTaskSource->Await())
        {
            LOG_ERROR("send http get request failure");
            return nullptr;
        }
        std::shared_ptr<HttpAsyncResponse> httpContent(new HttpAsyncResponse());
        std::shared_ptr<TaskSource<bool>> recvTaskSource(new TaskSource<bool>);
        netWorkThread.Invoke(&HttpRequestClient::ReceiveHttpContent, this, recvTaskSource, httpContent);
        return recvTaskSource->Await() ? httpContent : nullptr;
    }

    void HttpRequestClient::ReceiveHttpContent(std::shared_ptr<TaskSource<bool>> taskSource, std::shared_ptr<IHttpContent> httpContent)
    {
        AsioTcpSocket & tcpSocket = this->mSocket->GetSocket();
        asio::async_read(tcpSocket, this->mReadBuffer,
                         asio::transfer_at_least(1),[this, taskSource, httpContent]
            (const asio::error_code & code, size_t size)
        {
            if(code)
            {
                taskSource->SetResult(false);
                STD_ERROR_LOG(code.message());
                return;
            }
            HttpStatus httpStatus = httpContent->OnReceiveData(this->mReadBuffer);

            switch(httpStatus)
            {
                case HttpStatus::OK:
                    taskSource->SetResult(true);
                    break;
                case HttpStatus::CONTINUE:
                    this->ReceiveHttpContent(taskSource, httpContent);
                    break;
                default:
                    taskSource->SetResult(false);
            }
        });
    }

    void HttpRequestClient::SendByStream(std::shared_ptr<IHttpStream> httpStream, std::shared_ptr<TaskSource<bool>> taskSource)
    {
        asio::streambuf & stream = httpStream->GetStream();
        AsioTcpSocket & tcpSocket = this->mSocket->GetSocket();
        asio::async_write(tcpSocket, stream, [this, taskSource]
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

    std::shared_ptr<HttpAsyncResponse> HttpRequestClient::Post(const std::string &url, const std::string & content)
    {
        auto httpRequest = std::make_shared<HttpAsyncRequest>();
        if(!httpRequest->Post(url, content))
        {
            return nullptr;
        }
        return this->Request(httpRequest);
    }

    void HttpRequestClient::ConnectHost(const std::string & host, const std::string & port, std::shared_ptr<TaskSource<XCode>> taskSource)
    {
        AsioContext & context = this->mSocket->GetContext();
        std::shared_ptr<asio::ip::tcp::resolver> resolver(new asio::ip::tcp::resolver(context));
        std::shared_ptr<asio::ip::tcp::resolver::query> query(new asio::ip::tcp::resolver::query(host, port));
        resolver->async_resolve(*query, [this, resolver, query, taskSource]
            (const asio::error_code &err, asio::ip::tcp::resolver::iterator iterator)
        {
            if(err)
            {
                STD_ERROR_LOG(err.message());
                taskSource->SetResult(XCode::HttpNetWorkError);
                return;
            }
            AsioTcpSocket & tcpSocket = this->mSocket->GetSocket();
            asio::async_connect(tcpSocket, iterator, [taskSource, this]
                (const asio::error_code & code, asio::ip::tcp::resolver::iterator iter)
            {
                if(code)
                {
                    STD_ERROR_LOG(code.message());
                    taskSource->SetResult(XCode::HttpNetWorkError);
                    return;
                }
                this->mSocket->RefreshState();
                taskSource->SetResult(XCode::Successful);
            });
        });
    }

}
