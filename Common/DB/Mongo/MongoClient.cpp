//
// Created by mac on 2022/5/18.
//

#include "MongoClient.h"

namespace Mongo
{
	MongoClientContext::MongoClientContext(std::shared_ptr<SocketProxy> scoket)
		: Tcp::TcpContext(scoket)
	{
		this->mWriteLock = std::make_shared<CoroutineLock>();
		this->mConnectLock = std::make_shared<CoroutineLock>();
	}

	void MongoClientContext::OnConnect(const asio::error_code& error, int count)
	{
		if(error)
		{
#ifdef __DEBUG__
			CONSOLE_LOG_ERROR(error.message());
#endif
			this->mConnectTask.SetResult(false);
			return;
		}
		this->mConnectTask.SetResult(true);
	}

	void MongoClientContext::OnSendMessage(const asio::error_code& code, std::shared_ptr<ProtoMessage> message)
	{
		if (code)
		{
#ifdef __DEBUG__
			CONSOLE_LOG_ERROR(code.message());
#endif
			return;
		}
        this->mReadState == ReadType::HEAD;
        this->ReceiveMessage(sizeof(MongoHead));
        this->mWriteTask.SetResult(true);
	}

    void MongoClientContext::OnReceiveMessage(const asio::error_code &code, asio::streambuf &buffer)
    {
        std::iostream os(&buffer);

        if(this->mReadState == ReadType::HEAD)
        {
            this->mReadState = ReadType::BODY;
            this->mResponse = std::make_shared<MongoQueryResponse>();
            int length = this->mResponse->OnReceiveHead(buffer);
            this->ReceiveMessage(length - sizeof(MongoHead));
            return;
        }
        this->mResponse->OnReceiveBody(buffer);


        std::shared_ptr<Mongo::MongoQueryRequest> queryRequest
                = std::make_shared<Mongo::MongoQueryRequest>();
        queryRequest->collectionName = "ET.UserLevelData.$cmd";
        queryRequest->document.set("_id", 444);

        this->Send(queryRequest);
        if (code)
        {
#ifdef __DEBUG__
            CONSOLE_LOG_ERROR(code.message());
#endif
        }
    }

	bool MongoClientContext::StartConnect()
	{
		AutoCoroutineLock lock(this->mConnectLock);

		this->mConnectTask.Clear();
#ifdef ONLY_MAIN_THREAD
		this->Connect();
#else
		this->mNetworkThread.Invoke(&MongoClientContext::Connect, this);
#endif
		return this->mConnectTask.Await();
	}

	void MongoClientContext::SendMongoCommand(std::shared_ptr<Tcp::ProtoMessage> request)
	{
		AutoCoroutineLock lock(this->mWriteLock);
#ifdef ONLY_MAIN_THREAD
		this->Send(request);
#else
		this->mNetworkThread.Invoke(&MongoClientContext::Send, this, request);
#endif
		this->mWriteTask.Clear();
		if(!this->mWriteTask.Await())
		{
			LOG_ERROR("send mongo command error");
			return;
		}
		LOG_DEBUG("send mongo command successful");
	}

}