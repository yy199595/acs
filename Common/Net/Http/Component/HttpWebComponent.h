//
// Created by zmhy0073 on 2022/6/21.
//

#ifndef APP_HTTPWEBCOMPONENT_H
#define APP_HTTPWEBCOMPONENT_H

#include"Http/Common/Config.h"
#include"HttpListenComponent.h"
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
    class HttpMethodConfig;
	class HttpWebComponent final : public HttpListenComponent, public IServerRecord
    {
    public:
        HttpWebComponent();
    private:
		bool LateAwake() final;
		void OnRecord(json::w::Document& document) final;
	private:
		bool ReadHttpConfig();
		void OnReadHead(http::Request *request, http::Response *response) noexcept final;
		void OnMessage(http::Request * request, http::Response * response) noexcept final;
		void OnApi(const HttpMethodConfig* config, http::Request * request, http::Response * response) noexcept;
		void Invoke(const HttpMethodConfig* config, http::Request * request, http::Response * response) noexcept;
	private:
		HttpStatus OnNotFound(http::Request* request, http::Response* response) noexcept;
		HttpStatus AuthToken(const HttpMethodConfig* config, http::Request *request) noexcept;
		HttpStatus CreateHttpData(const HttpMethodConfig* config, http::Request * request) noexcept;
	public:
		bool AddRootDirector(const std::string & dir);
	private:
		http::Config mConfig;
		http::ContentFactory mFactory;
		std::vector<std::string> mRoots;
		class DispatchComponent * mDispatch;
		class CoroutineComponent * mCoroutine;
		std::unordered_map<std::string, std::string> mDefaultHeader;
		custom::HashMap<std::string, class HttpService *> mHttpServices;
		class IRequest<HttpMethodConfig, http::Request, http::Response> * mRecord;
	};
}


#endif //APP_HTTPWEBCOMPONENT_H
