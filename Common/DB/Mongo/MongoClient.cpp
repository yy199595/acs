//
// Created by mac on 2022/5/18.
//

#include "MongoClient.h"

namespace Mongo
{
	MongoClientContext::MongoClientContext(std::shared_ptr<SocketProxy> scoket)
		: Tcp::TcpContext(scoket)
	{
		this->SetBufferCount(10240, 10240 * 2);
		this->mWriteLock = std::make_shared<CoroutineLock>();
		this->mConnectLock = std::make_shared<CoroutineLock>();
	}

	void MongoClientContext::OnConnect(const asio::error_code& error)
	{
		if(error)
		{
#ifdef __DEBUG__
			CONSOLE_LOG_ERROR(error.message());
#endif
			std::move(this->mConnectTask)->SetResult(false);
			return;
		}
		std::move(this->mConnectTask)->SetResult(true);
	}

	bool MongoClientContext::OnRecvMessage(const asio::error_code& code, const char* message, size_t size)
	{
		return true;
	}

	void MongoClientContext::OnSendMessage(const asio::error_code& code, std::shared_ptr<ProtoMessage> message)
	{
		if(code)
		{
#ifdef __DEBUG__
			CONSOLE_LOG_ERROR(code.message());
#endif
			std::move(this->mWriteTask)->SetResult(false);
			return;
		}
		std::move(this->mWriteTask)->SetResult(true);
	}

	bool MongoClientContext::StartConnect()
	{
		AutoCoroutineLock lock(this->mConnectLock);
		if(this->IsOpen())
		{
			return true;
		}
		this->mConnectTask = std::make_shared<TaskSource<bool>>();
#ifdef ONLY_MAIN_THREAD
		this->Connect();
#else
		this->mNetworkThread.Invoke(&MongoClientContext::Connect, this);
#endif
		return this->mConnectTask->Await();
	}

	void MongoClientContext::SendMongoCommand(std::shared_ptr<MongoRequest> request)
	{
		AutoCoroutineLock lock(this->mWriteLock);
		this->mWriteTask = std::make_shared<TaskSource<bool>>();

#ifdef ONLY_MAIN_THREAD
		this->Send(request);
#else
		this->mNetworkThread.Invoke(&MongoClientContext::Send, this, request);
#endif
		if(!this->mWriteTask->Await())
		{
			LOG_ERROR("send mongo command error");
			return;
		}
		LOG_DEBUG("send mongo command successful");
#ifdef ONLY_MAIN_THREAD
		this->ReceiveHead();
#else
		this->mNetworkThread.Invoke(&MongoClientContext::ReceiveHead, this);
#endif
	}

}