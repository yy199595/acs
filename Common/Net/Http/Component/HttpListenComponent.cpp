//
// Created by zmhy0073 on 2022/8/11.
//

#include"XCode/XCode.h"
#include"Util/Zip/Zip.h"
#include"HttpListenComponent.h"
#include"Entity/Actor/App.h"
#include"Server/Config/CodeConfig.h"
#include"Http/Common/HttpResponse.h"
#include"Http/Client/SessionClient.h"
#include"Server/Component/ThreadComponent.h"

namespace joke
{
	HttpListenComponent::HttpListenComponent()
	{
		this->mSuccessCount = 0;
	}

    void HttpListenComponent::OnSecondUpdate(int tick)
	{
//        if(this->mHttpClients.Size() >0 )
//        {
//            CONSOLE_LOG_ERROR("size = {}", this->mHttpClients.Size());
//        }
    }

    bool HttpListenComponent::OnListen(tcp::Socket * socket)
	{
		if (this->mHttpClients.Size() >= MAX_HANDLE_HTTP_COUNT)
		{
			this->mWaitSockets.Push(socket);
			return true;
		}

        int sockId = this->mNumPool.BuildNumber();
        http::SessionClient* handlerClient = this->mClientPools.Pop();
		if (handlerClient == nullptr)
		{
			handlerClient = new http::SessionClient(this);
		}
		handlerClient->StartReceive(sockId, socket);
		this->mHttpClients.Add(sockId, handlerClient);
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
		tcp::Socket* tcpSocket = nullptr;
		http::SessionClient* handlerClient = nullptr;
		if (this->mHttpClients.Del(id, handlerClient))
		{
			if (code == XCode::Ok)
			{
				this->mSuccessCount++;
			}
			if (this->mWaitSockets.Pop(tcpSocket))
			{
				int sockId = this->mNumPool.BuildNumber();
				this->mHttpClients.Add(sockId, handlerClient);
				handlerClient->StartReceive(sockId, tcpSocket);
				return;
			}
			this->mClientPools.Push(handlerClient);
			return;
		}
		LOG_ERROR("not find http client : {} code = {}", id, code);
	}

	bool HttpListenComponent::Send(int id)
	{
		http::SessionClient * httpClient = nullptr;
		if(!this->mHttpClients.Get(id, httpClient))
		{
			LOG_ERROR("send message to {} fail", id);
			return false;
		}
		return httpClient->StartWriter();
	}

    bool HttpListenComponent::Send(int id, HttpStatus code)
    {
		http::SessionClient * httpClient = nullptr;
		if(!this->mHttpClients.Get(id, httpClient))
		{
			LOG_ERROR("send message to {} fail", id);
			return false;
		}
        return httpClient->StartWriter(code);
    }

	void HttpListenComponent::ClearClients()
	{
		auto iter = this->mHttpClients.Begin();
		for(; iter != this->mHttpClients.End(); iter++)
		{
			delete iter->second;
		}
		this->mClientPools.Clear();
	}

	std::string HttpListenComponent::RenderHtml(HttpStatus status)
	{
		std::vector<std::string> htmls;
		const char * error = HttpStatusToString(status);
		htmls.emplace_back("<!DOCTYPE html><html lang='en'><head> <meta charset='UTF-8'>");
		htmls.emplace_back("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">");
		htmls.emplace_back("<link rel=\"icon\" type=\"image/png\" href=\"https://lf16-fe.bytedgame.com/obj/gamefe-sg/toutiao/fe/game_interface_platform_fe/icon.png\">");
		htmls.emplace_back(fmt::format("<title>{}</title>", error));
		htmls.emplace_back("<style>body {font-family: 'Arial', sans-serif;background-color: #f4f4f4;color: #333;text-align: center; padding: 50px; margin: 0;}");
		htmls.emplace_back(".container {max-width: 600px;margin: 0 auto; }");
		htmls.emplace_back("h1 {\"font-size: 8em;color: #e74c3c;margin-bottom: 10px;}");
		htmls.emplace_back(" p {font-size: 1.5em;color: #888;margin-top: 0;}a {color: #3498db;text-decoration: none;}a:hover {"
						   "text-decoration: underline;}.illustration {margin-top: 50px;max-width: 100%;}");
		htmls.emplace_back("</style></head><body><div class=\"container\">");
		htmls.emplace_back(fmt::format("<h1>{}</h1>", (int)status));
		htmls.emplace_back(fmt::format("<p>{}</p>", error));
		htmls.emplace_back("<img class=\"illustration\" src=\"https://placekitten.com/800/400\" alt=\"Lost Kitten\">");
		htmls.emplace_back("</div></body></html>");

		std::stringstream ss;
		for(const std::string & str : htmls)
		{
			ss << str;
		}
		return ss.str();
	}
}