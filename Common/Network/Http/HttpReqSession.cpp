//
// Created by zmhy0073 on 2021/10/29.
//

#include"HttpReqSession.h"
#include"Http/Response/HttpRespTask.h"

#include <Core/App.h>
#include <Network/NetworkHelper.h>
#include <Scene/ThreadPoolComponent.h>
namespace GameKeeper
{
	HttpReqSession::HttpReqSession(NetWorkThread & thread)
        : mThread(thread)
	{
		this->mQuery = nullptr;
		this->mResolver = nullptr;
	}

	HttpReqSession::~HttpReqSession()
    {		
        delete this->mQuery;
        delete this->mResolver;
    }

	void HttpReqSession::StartConnectHost(const std::string & host, const std::string & port)
    {
        this->mHost = host;
        this->mPort = port;
        delete this->mSocketProxy;
        this->mSocketProxy = new SocketProxy(this->mThread, "HttpGet");

        if (this->mThread.IsCurrentThread())
        {
            this->Resolver();
            return;
        }
        this->mThread.Invoke(&HttpReqSession::Resolver, this);
    }

	bool HttpReqSession::OnReceiveHead(asio::streambuf & buf)
    {
        this->mHttpRespTask->OnReceiveHead(buf);
        return true;
    }

    bool HttpReqSession::OnReceiveBody(asio::streambuf &buf)
    {
        return this->mHttpRespTask->OnReceiveBody(buf);
    }

    void HttpReqSession::OnComplete(XCode code)
    {
        MainTaskScheduler &taskScheduler = App::Get().GetTaskScheduler();
        taskScheduler.Invoke(&HttpRespTask::OnComplete, this->mHttpRespTask.get(), code);
    }

	void HttpReqSession::Resolver()
    {
        delete this->mQuery;
        delete this->mResolver;
		asio::io_context &ctx = mSocketProxy->GetContext();
        this->mResolver = new asio::ip::tcp::resolver(ctx);
        this->mQuery = new asio::ip::tcp::resolver::query(this->mHost, this->mPort);
        this->mResolver->async_resolve(*this->mQuery, [this](
                const asio::error_code &err, asio::ip::tcp::resolver::iterator iterator)
        {
            if (err)
            {
                this->mHttpRespTask->OnComplete(XCode::HttpNetWorkError);
                LOG_ERROR("resolver " << this->mHost << ":" << this->mPort << " failure : " << err.message());
                return;
            }
			AsioTcpSocket & socket = this->mSocketProxy->GetSocket();
			asio::async_connect(socket, std::move(iterator),
				std::bind(&HttpReqSession::ConnectHandler, this, args1));
        });
    }

    void HttpReqSession::OnWriterAfter(XCode code)
    {
        if (code != XCode::Successful)
        {
            this->mHttpRespTask->OnComplete(code);
            return;
        }
        this->StartReceiveHead();
    }

	void HttpReqSession::ConnectHandler(const asio::error_code & err)
    {
        if (err)
        {
            this->mHttpRespTask->OnComplete(XCode::HttpNetWorkError);
            LOG_ERROR("connect " << this->mHost << ":" << this->mPort << " failure : " << err.message());
            return;
        }
		AsioTcpSocket & socket = this->mSocketProxy->GetSocket();
        this->mAddress = socket.remote_endpoint().address().to_string()
                         + ":" + std::to_string(socket.remote_endpoint().port());
        LOG_DEBUG("connect to " << this->mHost << ":" << this->mPort << " successful");
        this->StartSendHttpMessage();
    }


    void HttpReqSession::WriterToBuffer(std::ostream & os)
    {
        this->mHttpRequest->WriteToSendBuffer(os);
    }
}