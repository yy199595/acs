//
// Created by zmhy0073 on 2022/6/21.
//

#ifndef APP_HTTPWEBCOMPONENT_H
#define APP_HTTPWEBCOMPONENT_H

#include"Http/Common/Config.h"
#include"HttpListenComponent.h"
#include"Rpc/Client/Message.h"
#include"Cache/Common/LruCache.h"
#include"Http/Common/ContentType.h"
namespace http
{
	class Content;
    class Request;
    class Response;
}

namespace http
{
	struct Token
	{
		int UserId = 0;
		int ClubId = 0;
		int Permission = 0;
		long long ExpTime = 0;
	};
}

namespace acs
{
	class RpcMethodConfig;
    class HttpMethodConfig;
	typedef IRequest<HttpMethodConfig, http::Request, http::Response> HttpHandlerComponent;
	class HttpWebComponent final : public HttpListenComponent, public IServerRecord
    {
    public:
        HttpWebComponent();
    private:
		bool LateAwake() final;
		void OnRecord(json::w::Document& document) final;
	private:
		bool ReadHttpConfig();
		void OnApi(const RpcMethodConfig* config, http::Request * request, http::Response * response);
		void OnApi(const HttpMethodConfig* config, http::Request * request, http::Response * response);
		void Invoke(const HttpMethodConfig* config, http::Request * request, http::Response * response);
	private:
		void OnReadHead(http::Request *request, http::Response *response) final;
		void OnMessage(http::Request * request, http::Response * response) final;
	private:
		HttpStatus OnNotFound(http::Request* request, http::Response* response);
		HttpStatus AuthToken(const HttpMethodConfig* config, http::Request *request);
		HttpStatus CreateHttpData(const HttpMethodConfig* config, http::Request * request);
	public:
		bool AddRootDirector(const std::string & dir);
	private:
		http::Config mConfig;
		http::ContentFactory mFactory;
		std::vector<std::string> mRoots;
		class DispatchComponent * mDispatch;
		class CoroutineComponent * mCorComponent;
		std::vector<HttpHandlerComponent *> mRecordComponents;
		std::unordered_map<std::string, std::string> mDefaultHeader;
		custom::HashMap<std::string, class RpcService *> mRpcServices;
		custom::HashMap<std::string, class HttpService *> mHttpServices;
	};
}


#endif //APP_HTTPWEBCOMPONENT_H
