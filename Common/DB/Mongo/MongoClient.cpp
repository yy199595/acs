//
// Created by mac on 2022/5/18.
//

#include"MongoClient.h"
#include"Component/Mongo/MongoComponent.h"

namespace Mongo
{
	MongoClientContext::MongoClientContext(std::shared_ptr<SocketProxy> scoket,
			const Mongo::Config& config, MongoComponent* component)
		: Tcp::TcpContext(scoket), mConfig(config), mMongoComponent(component)
	{

	}

	void MongoClientContext::OnConnect(const asio::error_code& error, int count)
	{
		if(error)
		{
#ifdef __DEBUG__
			CONSOLE_LOG_ERROR(error.message());
#endif
			this->mTimer = std::make_shared<asio::steady_timer>(this->mNetworkThread, std::chrono::seconds(3));
			this->mTimer->async_wait(std::bind(&MongoClientContext::Connect, this));
		}
		else
		{
			if(this->mTimer != nullptr)
			{
				this->mTimer->cancel();
				std::move(this->mTimer);
			}
			if(!this->mCommands.empty())
			{
				this->Send(this->mCommands.front());
				this->mCommands.pop_front();
			}
		}
	}

	void MongoClientContext::OnSendMessage(const asio::error_code& code, std::shared_ptr<ProtoMessage> message)
	{
		if (code)
		{
#ifdef __DEBUG__
			CONSOLE_LOG_ERROR(code.message());
#endif
			this->Connect();
		}
		else
		{
			this->mReadState == ReadType::HEAD;
			if (!this->mCommands.empty())
			{
				this->Send(this->mCommands.front());
				this->mCommands.pop_front();
			}
			this->ReceiveMessage(sizeof(MongoHead));
		}
	}

    void MongoClientContext::OnReceiveMessage(const asio::error_code &code, asio::streambuf &buffer)
    {
		if (code)
		{
#ifdef __DEBUG__
			CONSOLE_LOG_ERROR(code.message());
#endif
			return;
		}
        std::iostream os(&buffer);
        if(this->mReadState == ReadType::HEAD)
        {
            this->mReadState = ReadType::BODY;
            int length = mMongoResponse.OnReceiveHead(buffer);
            this->ReceiveMessage(length - sizeof(MongoHead));
            return;
        }
        this->mMongoResponse.OnReceiveBody(buffer);
		const MongoHead & mongoHead = this->mMongoResponse.GetHead();
		std::shared_ptr<_bson::bsonobj> res = this->mMongoResponse.GetObject();
#ifdef ONLY_MAIN_THREAD
		this->mMongoComponent->OnResponse(mongoHead.responseTo, res);
#else
		MainTaskScheduler & taskScheduler = App::Get()->GetTaskScheduler();
		taskScheduler.Invoke(&MongoComponent::OnResponse, this->mMongoComponent, mongoHead.responseTo, res);
#endif
    }

	void MongoClientContext::SendMongoCommand(std::shared_ptr<Tcp::ProtoMessage> request)
	{
#ifdef ONLY_MAIN_THREAD
		this->AddToCommandQueue(request);
#else
		this->mNetworkThread.Invoke(&MongoClientContext::AddToCommandQueue, this, request);
#endif
	}

	void MongoClientContext::AddToCommandQueue(std::shared_ptr<Tcp::ProtoMessage> request)
	{
		if(this->mCommands.empty())
		{
			this->Send(request);
		}
		this->mCommands.emplace_back(request);
	}

}