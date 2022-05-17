//
// Created by zmhy0073 on 2022/1/19.
//

#include"HttpHandlerClient.h"

namespace Sentry
{
	HttpHandlerClient::HttpHandlerClient(std::shared_ptr<SocketProxy> socketProxy)
	{
		this->mSocket = socketProxy;
		this->mHttpResponse = std::make_shared<HttpHandlerResponse>();
		this->mHttpRequest = std::make_shared<HttpHandlerRequest>(socketProxy->GetAddress());
	}

	std::shared_ptr<HttpHandlerRequest> HttpHandlerClient::Read()
	{
		this->mReadTask = std::make_shared<TaskSource<bool>>();
#ifdef ONLY_MAIN_THREAD
		this->ReadData();
#else
		IAsioThread & netWorkThread = this->mSocket->GetThread();
		netWorkThread.Invoke(&HttpHandlerClient::ReadData, this);
#endif
		return this->mReadTask->Await() ? this->mHttpRequest : nullptr;
	}

	bool HttpHandlerClient::Writer(HttpStatus code, Json::Writer& jsonWriter)
	{
		this->mHttpResponse->Write(code, jsonWriter);
		this->mWriteTask = std::make_shared<TaskSource<bool>>();
#ifdef ONLY_MAIN_THREAD
		this->WriteData();
#else
		IAsioThread& netWorkThread = this->mSocket->GetThread();
		netWorkThread.Invoke(&HttpHandlerClient::WriteData, this);
#endif
		return this->mWriteTask->Await();
	}

	void HttpHandlerClient::WriteData()
	{
		asio::streambuf& streambuf = this->mHttpResponse->GetStream();
		AsioTcpSocket& tcpSocket = this->mSocket->GetSocket();
		std::shared_ptr<HttpHandlerClient> self = this->shared_from_this();
		asio::async_write(tcpSocket, streambuf, [this, self]
			(const asio::error_code& code, size_t size)
		{
			if (code)
			{
				this->mWriteTask->SetResult(false);
				return;
			}
			this->mWriteTask->SetResult(true);
		});
	}

	void HttpHandlerClient::ReadData()
	{
		AsioTcpSocket& tcpSocket = this->mSocket->GetSocket();
		asio::async_read(tcpSocket, this->mStreamBuffer, asio::transfer_at_least(1),
			[this](const asio::error_code& code, size_t size)
			{
				if (code)
				{
					this->mReadTask->SetResult(false);
					return;
				}
				HttpStatus httpCode = this->mHttpRequest->OnReceiveData(this->mStreamBuffer);
				switch (httpCode)
				{
				case HttpStatus::CONTINUE:
					this->ReadData();
					break;
				case HttpStatus::OK:
					this->mReadTask->SetResult(true);
					break;
				default:
					this->mReadTask->SetResult(false);
					break;
				}
			});
	}
}