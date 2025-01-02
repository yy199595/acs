//
// Created by 64658 on 2025/1/2.
//

#include "WebSocketClient.h"
#include "XCode/XCode.h"
#include "Http/Common/HttpRequest.h"
namespace ws
{
	RequestClient::RequestClient(int id, ws::Component* component, Asio::Context& main)
			: tcp::Client(1024 * 10), mSockId(id), mIsHttp(true), mComponent(component), mMainContext(main)
	{
		this->mHttpRequest = nullptr;
	}

	void RequestClient::OnSendMessage()
	{
		if(!this->mWaitSendMessage.empty())
		{
			delete this->mWaitSendMessage.front();
			this->mWaitSendMessage.pop();
			if(!this->mWaitSendMessage.empty())
			{
				ws::Message * message = this->mWaitSendMessage.front();
				{
					this->Write(*message);
				}
			}
		}
	}

	void RequestClient::OnConnect(bool result, int count)
	{
		if(!result)
		{
			this->Close(XCode::NetConnectFailure);
			return;
		}
		this->mHttpRequest = new http::Request();
		{

		}
	}

	void RequestClient::Close(int code)
	{
		if(this->mSockId == 0)
		{
			return;
		}

		this->StopTimer();
		this->mSocket->Close();
		std::shared_ptr<Client> self = this->shared_from_this();
		asio::post(this->mMainContext, [self, this, code, id = this->mSockId]()
		{
			while(!this->mWaitSendMessage.empty())
			{
				ws::Message * message = this->mWaitSendMessage.front();
				this->mComponent->OnSendFailure(id, message);
				this->mWaitSendMessage.pop();
			}
			this->mComponent->OnClientError(id, code);
		});
		this->mSockId = 0;
	}

	void RequestClient::StartWrite(ws::Message* message)
	{
		Asio::Context & context = this->mSocket->GetContext();
		asio::post(context, [this, self = this->shared_from_this(), message] ()
		{
			this->mWaitSendMessage.emplace(message);
			if(this->mWaitSendMessage.size() == 1)
			{
				this->Write(*message);
			}
		});
	}

	void RequestClient::OnReadError(const Asio::Code& code)
	{
		this->Close(XCode::NetReadFailure);
	}

	void RequestClient::OnSendMessage(const Asio::Code& code)
	{
		this->Close(XCode::NetSendFailure);
	}

	void RequestClient::OnReceiveLine(std::istream& readStream, size_t size)
	{

	}

	void RequestClient::OnReceiveMessage(std::istream& readStream, size_t size, const asio::error_code& code)
	{

	}
}