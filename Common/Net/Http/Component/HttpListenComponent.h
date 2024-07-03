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
#define MAX_HANDLE_HTTP_COUNT 100
namespace http
{
	class SessionClient;
}

namespace joke
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
                                public IRpc<http::Request, http::Response>, public ISecondUpdate
    {
	public:
		HttpListenComponent();
	protected:
		bool Send(int fd);
		void ClearClients();
		bool ReadMessageBody(int id);
		void StartClose(int id, int) final;
		bool Send(int fd, HttpStatus code);
	private:
        void OnSecondUpdate(int tick) final;
		bool OnListen(tcp::Socket * socket) final;
		void OnCloseSocket(int, int code) final;
		std::string RenderHtml(HttpStatus status);
	protected:
		unsigned int mSuccessCount; //成功次数
        math::NumberPool<int> mNumPool;
		custom::Queue<tcp::Socket *> mWaitSockets;
		custom::ArrayPool<http::SessionClient, 100> mClientPools;
		custom::HashMap<int, http::SessionClient *> mHttpClients;
	};
}


#endif //APP_HTTPLISTENCOMPONENT_H
