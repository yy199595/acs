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
		http::SessionClient* handlerClient = new http::SessionClient(this);
		{
			handlerClient->StartReceive(sockId, socket);
			this->mHttpClients.Add(sockId, handlerClient);
			LOG_DEBUG("connect sock id => {}", sockId);
		}
		return true;
	}

	void HttpListenComponent::StartClose(int id, int code)
	{
		http::SessionClient* handlerClient = nullptr;
		if (!this->mHttpClients.Get(id, handlerClient))
		{
			return;
		}
		handlerClient->StartClose(code);
	}

	bool HttpListenComponent::ReadMessageBody(int id)
	{
		http::SessionClient* handlerClient = nullptr;
		if (!this->mHttpClients.Get(id, handlerClient))
		{
			return false;
		}
		handlerClient->StartReceiveBody();
		return true;
	}

	void HttpListenComponent::OnCloseSocket(int id, int code)
	{
		LOG_WARN("close sock id => {}", id);
		http::SessionClient* handlerClient = nullptr;
		if (!this->mHttpClients.Del(id, handlerClient))
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
		delete handlerClient;
		tcp::Socket* tcpSocket = nullptr;
		if (this->mWaitSockets.Pop(tcpSocket))
		{
			int sockId = this->mNumPool.BuildNumber();
			http::SessionClient* newClient = new http::SessionClient(this);
			{
				this->mHttpClients.Add(sockId, newClient);
				newClient->StartReceive(sockId, tcpSocket, 0);
			}	
		}
	}

	bool HttpListenComponent::SendResponse(int id)
	{
		http::SessionClient* httpClient = nullptr;
		if (!this->mHttpClients.Get(id, httpClient))
		{
			LOG_ERROR("send message to {} fail", id);
			return false;
		}
		return httpClient->StartWriter();
	}

	bool HttpListenComponent::SendResponse(int id, HttpStatus code)
	{
		http::SessionClient* httpClient = nullptr;
		if (!this->mHttpClients.Get(id, httpClient))
		{
			LOG_ERROR("send message to {} fail", id);
			return false;
		}
		return httpClient->StartWriter(code);
	}

	void HttpListenComponent::ClearClients()
	{
		auto iter = this->mHttpClients.Begin();
		for (; iter != this->mHttpClients.End(); iter++)
		{
			delete iter->second;
		}
		
	}
}