//
// Created by yjz on 2022/1/19.
//

#ifndef GAMEKEEPER_HTTPREQUESTCLIENT_H
#define GAMEKEEPER_HTTPREQUESTCLIENT_H
#include<Async/TaskSource.h>
#include"HttpAsyncRequest.h"
#include"Network/TcpContext.h"
namespace Sentry
{
	class HttpRequestClient : public Tcp::TcpContext
	{
	 public:
		HttpRequestClient(std::shared_ptr<SocketProxy> socketProxy);
	 public:
		std::shared_ptr<HttpAsyncResponse> Get(const std::string& url);
		std::shared_ptr<HttpAsyncResponse> Post(const std::string& url, Json::Writer& json);
		std::shared_ptr<HttpAsyncResponse> Request(std::shared_ptr<HttpAsyncRequest> request);
		std::shared_ptr<HttpAsyncResponse> Post(const std::string& url, const std::string& content);
	private:
		void ConnectHost(const std::string& host, const std::string& port);
		void ReceiveHttpContent( std::shared_ptr<IHttpContent> httpContent);
		void OnSendMessage(const asio::error_code &code, std::shared_ptr<Tcp::ProtoMessage> message) final;
	 private:
		TaskSource<bool> mHttpTask;
		asio::streambuf mReadBuffer;
	};
}
#endif //GAMEKEEPER_HTTPREQUESTCLIENT_H
