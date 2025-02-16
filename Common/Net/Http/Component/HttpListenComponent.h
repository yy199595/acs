//
// Created by zmhy0073 on 2022/8/11.
//

#ifndef APP_HTTPLISTENCOMPONENT_H
#define APP_HTTPLISTENCOMPONENT_H
#include"Http/Client/Http.h"
#include"Core/Map/HashMap.h"
#include"Core/Queue/Queue.h"
#include"Core/Pool/ArrayPool.h"
#include"Server/Component/ListenerComponent.h"
namespace http
{
	class Request;
	class Response;
}

//允许同时处理多少个http请求
#define MAX_HANDLE_HTTP_COUNT 0
namespace http
{
	class Content;
	class Session;
}

namespace acs
{
	enum MessageStatus
	{
		ReadBody = 1,
		Response = 2,
		Pause = 3,
		Done = 4,
		Close = 5,
		Shield = 6
	};
	class HttpListenComponent : public Component, public ITcpListen,
                                public IRpc<http::Request, http::Response>
    {
	public:
		HttpListenComponent();
	protected:
		void ClearClients();
		bool ReadMessageBody(int id);
		void StartClose(int id, int) final;
		bool ReadMessageBody(int id, std::unique_ptr<http::Content> data);
	protected:
		bool SendResponse(int fd);
		bool SendResponse(int fd, HttpStatus code);
		bool SendResponse(int fd, HttpStatus code, std::unique_ptr<http::Content> data);
	private:
		bool OnListen(tcp::Socket * socket) noexcept final;
		void OnClientError(int id, int code) final;
	protected:
		unsigned int mSuccessCount; //成功次数
		unsigned int mFailureCount; //失败次数
        math::NumberPool<int> mNumPool;
		std::unordered_map<int, std::shared_ptr<http::Session>> mHttpClients;
	};
}


#endif //APP_HTTPLISTENCOMPONENT_H
