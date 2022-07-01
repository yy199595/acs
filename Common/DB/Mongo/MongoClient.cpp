//
// Created by mac on 2022/5/18.
//

#include"MongoClient.h"
#include"Component/Mongo/MongoComponent.h"

#include"Util/MD5.h"
#include"Util/StringHelper.h"
#include"Util/Base64Helper.h"
namespace Mongo
{
	MongoClientContext::MongoClientContext(std::shared_ptr<SocketProxy> scoket,
			const Mongo::Config& config, MongoComponent* component, int index)
		: Tcp::TcpContext(scoket), mConfig(config), mMongoComponent(component), mIndex(index)
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
			this->mCommands.push_front(this->AuthRequest1());
		}
		else
		{
			this->mCommands.pop_front();
			this->mReadState == ReadType::HEAD;
			this->ReceiveMessage(sizeof(MongoHead));
		}
	}

    void MongoClientContext::OnReceiveMessage(const asio::error_code &code, asio::streambuf &buffer)
    {
		if (code)
		{
#ifdef ONLY_MAIN_THREAD
			this->mMongoComponent->OnClientError(this->mIndex, XCode::NetReceiveFailure);
#else
			MainTaskScheduler & taskScheduler = App::Get()->GetTaskScheduler();
			taskScheduler.Invoke(&MongoComponent::OnClientError, this->mMongoComponent, this->mIndex, XCode::NetReceiveFailure);
#endif
			CONSOLE_LOG_ERROR(code.message());
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
		const MongoHead & mongoHead = this->mMongoResponse.GetHead();
		std::shared_ptr<Bson::ReaderDocument> res = this->mMongoResponse.OnReceiveBody(buffer);
		std::string json;
		res->WriterToJson(json);

		if(mongoHead.requestID == 1) //第一次握手
		{

		}
		else
		{
#ifdef ONLY_MAIN_THREAD
			this->mMongoComponent->OnResponse(mongoHead.responseTo, res);
#else
			MainTaskScheduler& taskScheduler = App::Get()->GetTaskScheduler();
			taskScheduler.Invoke(&MongoComponent::OnResponse, this->mMongoComponent, mongoHead.responseTo, res);
#endif
		}

		if (!this->mCommands.empty())
		{
			this->Send(this->mCommands.front());
		}
		this->mReadState = ReadType::HEAD;
	}

	void MongoClientContext::PushMongoCommand(std::shared_ptr<Tcp::ProtoMessage> request)
	{
#ifdef ONLY_MAIN_THREAD
		this->PushCommand(request, front);
#else
		this->mNetworkThread.Invoke(&MongoClientContext::PushCommand, this, request);
#endif
	}

	void MongoClientContext::PushCommand(std::shared_ptr<Tcp::ProtoMessage> request)
	{
		if(this->mCommands.empty())
		{
			this->Send(request);
		}
		this->mCommands.push_back(request);
	}

	std::shared_ptr<Mongo::MongoQueryRequest> MongoClientContext::AuthRequest1()
	{
		std::string random = Helper::String::RandomString(8);
		std::string nonce = Helper::Base64::Base64Encode(random);
		std::shared_ptr<Mongo::MongoQueryRequest> request(new MongoQueryRequest());
		std::string firstBare = fmt::format("n={0}user,r={1}", this->mConfig.mUser, nonce);
		std::string payload = Helper::Base64::Base64Encode(fmt::format("n,,{0}", firstBare));

		request->header.requestID = 1;
		request->collectionName = this->mConfig.mDb + ".$cmd";
		request->document.Add("saslStart", 1);
		request->document.Add("autoAuthorize", 1);
		request->document.Add("mechanism", "SCRAM-SHA-1");
		request->document.Add("payload", payload);
		return request;
	}

}