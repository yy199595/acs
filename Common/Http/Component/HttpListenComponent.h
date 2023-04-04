//
// Created by zmhy0073 on 2022/8/11.
//

#ifndef APP_HTTPLISTENCOMPONENT_H
#define APP_HTTPLISTENCOMPONENT_H

#include<queue>
#include<unordered_map>
#include"Http/Client/Http.h"
#include"Server/Component/TcpListenerComponent.h"
namespace Http
{
	class Request;
	class IResponse;
	class DataResponse;
}
namespace Sentry
{
    class HttpHandlerClient;
    class HttpListenComponent : public TcpListenerComponent
    {
    public:
        void ClosetHttpClient(const std::string & address);
        void OnListen(std::shared_ptr<SocketProxy> socket) final;
        virtual bool OnDelClient(const std::string& address) = 0;
        virtual void OnRequest(std::shared_ptr<Http::Request> request) = 0;
    protected:
		void ClearClients();
        bool Send(const std::string& address, HttpStatus code);
        bool Send(const std::string& address, const std::string & str);
		bool Send(const std::string& address, std::shared_ptr<Http::IResponse> response);
		bool SendFile(const std::string& address, const std::string & type, const std::string & path);
    private:     
        std::queue<std::shared_ptr<HttpHandlerClient>> mClientPools;
        std::unordered_map<std::string, std::shared_ptr<HttpHandlerClient>> mHttpClients;
    };
}


#endif //APP_HTTPLISTENCOMPONENT_H
