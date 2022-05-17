//
// Created by zmhy0073 on 2022/1/19.
//

#ifndef GAMEKEEPER_HTTPHANDLERCLIENT_H
#define GAMEKEEPER_HTTPHANDLERCLIENT_H
#include"HttpAsyncRequest.h"
#include"Async/TaskSource.h"
#include"Network/SocketProxy.h"
#include"Json/JsonWriter.h"
namespace Sentry
{
	class HttpHandlerClient : public std::enable_shared_from_this<HttpHandlerClient>
	{
	 public:
		HttpHandlerClient(std::shared_ptr<SocketProxy> socketProxy);
	 public:
		std::shared_ptr<HttpHandlerRequest> Read();
		bool Writer(HttpStatus code, Json::Writer& jsonWriter);
	 private:
		void ReadData();
		void WriteData();
	 private:
		asio::streambuf mStreamBuffer;
		std::shared_ptr<SocketProxy> mSocket;
		std::shared_ptr<TaskSource<bool>> mReadTask;
		std::shared_ptr<TaskSource<bool>> mWriteTask;
		std::shared_ptr<HttpHandlerRequest> mHttpRequest;
		std::shared_ptr<HttpHandlerResponse> mHttpResponse;
	};
}
#endif //GAMEKEEPER_HTTPHANDLERCLIENT_H
