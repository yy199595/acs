//
// Created by zmhy0073 on 2022/8/11.
//

#include"XCode/XCode.h"
#include"Util/Tools/Zip.h"
#include"HttpListenComponent.h"
#include"Entity/Actor/App.h"
#include"Server/Config/CodeConfig.h"
#include"Http/Common/HttpResponse.h"
#include"Http/Client/Session.h"
#include"Server/Component/ThreadComponent.h"

#include "Core/System/System.h"

namespace acs
{
	HttpListenComponent::HttpListenComponent()
	{
		this->mSuccessCount = 0;
		this->mFailureCount = 0;
	}

	bool HttpListenComponent::OnListen(tcp::Socket* socket) noexcept
	{
		int sockId = this->mNumPool.BuildNumber();
		Asio::Context & io = this->mApp->GetContext();
		std::shared_ptr<http::Session> handlerClient = std::make_shared<http::Session>(this, io);
		{
			handlerClient->StartReceive(sockId, socket);
			this->mHttpClients.emplace(sockId, handlerClient);
			//LOG_DEBUG("connect sock id => {}", sockId);
		}
		return true;
	}

	void HttpListenComponent::StartClose(int id, int code)
	{
		auto iter = this->mHttpClients.find(id);
		if(iter != this->mHttpClients.end())
		{
			iter->second->StartClose(code);
			this->mHttpClients.erase(iter);
		}
	}

	bool HttpListenComponent::ReadMessageBody(int id)
	{
		auto iter = this->mHttpClients.find(id);
		if(iter == this->mHttpClients.end())
		{
			return false;
		}
		iter->second->StartReceiveBody();
		return true;
	}

	bool HttpListenComponent::ReadMessageBody(int id, std::unique_ptr<http::Content> data)
	{
		auto iter = this->mHttpClients.find(id);
		if(iter == this->mHttpClients.end())
		{
			return false;
		}
		iter->second->StartReceiveBody(std::move(data));
		return true;
	}

	void HttpListenComponent::OnClientError(int id, int code)
	{
		//LOG_WARN("close sock id => {}", id);
		auto iter = this->mHttpClients.find(id);
		if (iter != this->mHttpClients.end())
		{
			this->mHttpClients.erase(iter);
		}
		if(code == XCode::Ok) {
			this->mSuccessCount++;
			return;
		}
		this->mFailureCount++;
	}

	bool HttpListenComponent::SendResponse(int id)
	{
		auto iter = this->mHttpClients.find(id);
		if(iter == this->mHttpClients.end())
		{
			LOG_ERROR("send message to {} fail", id);
			return false;
		}
		return iter->second->StartWriter();
	}

	bool HttpListenComponent::SendResponse(int id, HttpStatus code)
	{
		auto iter = this->mHttpClients.find(id);
		if(iter == this->mHttpClients.end())
		{
			LOG_ERROR("send message to {} fail", id);
			return false;
		}
		return iter->second->StartWriter(code);
	}

	bool HttpListenComponent::SendResponse(int id, HttpStatus code, std::unique_ptr<http::Content> data)
	{
		auto iter = this->mHttpClients.find(id);
		if(iter == this->mHttpClients.end())
		{
			LOG_ERROR("send message to {} fail", id);
			return false;
		}
		return iter->second->StartWriter(code, std::move(data));
	}

	void HttpListenComponent::ClearClients()
	{
		auto iter = this->mHttpClients.begin();
		for (; iter != this->mHttpClients.end(); iter++)
		{
			iter->second->StartClose(XCode::Ok);
		}
		this->mHttpClients.clear();
	}
}