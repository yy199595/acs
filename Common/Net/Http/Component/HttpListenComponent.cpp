//
// Created by zmhy0073 on 2022/8/11.
//

#include"XCode/XCode.h"
#include"Util/Tools/Zip.h"
#include"HttpListenComponent.h"
#include"Entity/Actor/App.h"
#include"Server/Config/CodeConfig.h"
#include"Http/Common/HttpResponse.h"
#include"Http/Client/SessionClient.h"
#include"Server/Component/ThreadComponent.h"

#include "Core/System/System.h"

namespace acs
{
	HttpListenComponent::HttpListenComponent()
	{
		this->mSuccessCount = 0;
		this->mFailureCount = 0;
	}

	bool HttpListenComponent::OnListen(tcp::Socket* socket)
	{
		if (this->mHttpClients.Size() >= MAX_HANDLE_HTTP_COUNT)
		{
			this->mWaitSockets.Push(socket);
			return true;
		}

		int sockId = this->mNumPool.BuildNumber();
		Asio::Context & io = this->mApp->GetContext();
		std::shared_ptr<http::SessionClient> handlerClient = std::make_shared<http::SessionClient>(this, io);
		{
			handlerClient->StartReceive(sockId, socket);
			this->mHttpClients.Add(sockId, handlerClient);
			//LOG_DEBUG("connect sock id => {}", sockId);
		}
		return true;
	}

	void HttpListenComponent::StartClose(int id, int code)
	{
		std::shared_ptr<http::SessionClient> handlerClient;
		if (this->mHttpClients.Del(id, handlerClient))
		{
			handlerClient->StartClose(code);
		}
	}

	bool HttpListenComponent::ReadMessageBody(int id)
	{
		std::shared_ptr<http::SessionClient> handlerClient;
		if (!this->mHttpClients.Get(id, handlerClient))
		{
			return false;
		}
		handlerClient->StartReceiveBody();
		return true;
	}

	bool HttpListenComponent::ReadMessageBody(int id, std::unique_ptr<http::Content> data)
	{
		std::shared_ptr<http::SessionClient> handlerClient;
		if (!this->mHttpClients.Get(id, handlerClient))
		{
			return false;
		}
		handlerClient->StartReceiveBody(std::move(data));
		return true;
	}

	void HttpListenComponent::OnClientError(int id, int code)
	{
		//LOG_WARN("close sock id => {}", id);
		if (!this->mHttpClients.Del(id))
		{
			LOG_ERROR("not find http client : {} code = {}", id, code);
			return;
		}

		if (code == XCode::Ok)
		{
			this->mSuccessCount++;
		}
		else
		{
			this->mFailureCount++;
		}
		tcp::Socket* tcpSocket = nullptr;
		if (this->mWaitSockets.Pop(tcpSocket))
		{
			int sockId = this->mNumPool.BuildNumber();
			Asio::Context & io = this->mApp->GetContext();
			std::shared_ptr<http::SessionClient> newClient = std::make_shared<http::SessionClient>(this, io);
			{
				this->mHttpClients.Add(sockId, newClient);
				newClient->StartReceive(sockId, tcpSocket, 0);
			}	
		}
	}

	bool HttpListenComponent::SendResponse(int id)
	{
		std::shared_ptr<http::SessionClient> httpClient;
		if (!this->mHttpClients.Get(id, httpClient))
		{
			LOG_ERROR("send message to {} fail", id);
			return false;
		}
		return httpClient->StartWriter();
	}

	bool HttpListenComponent::SendResponse(int id, HttpStatus code)
	{
		std::shared_ptr<http::SessionClient> httpClient;
		if (!this->mHttpClients.Get(id, httpClient))
		{
			LOG_ERROR("send message to {} fail", id);
			return false;
		}
		return httpClient->StartWriter(code);
	}

	bool HttpListenComponent::SendResponse(int id, HttpStatus code, std::unique_ptr<http::Content> data)
	{
		std::shared_ptr<http::SessionClient> httpClient;
		if (!this->mHttpClients.Get(id, httpClient))
		{
			LOG_ERROR("send message to {} fail", id);
			return false;
		}
		return httpClient->StartWriter(code, std::move(data));
	}

	void HttpListenComponent::ClearClients()
	{
		auto iter = this->mHttpClients.Begin();
		for (; iter != this->mHttpClients.End(); iter++)
		{
			iter->second->StartClose(XCode::Ok);
		}
		this->mHttpClients.Clear();
	}
}