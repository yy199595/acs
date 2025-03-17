//
// Created by zmhy0073 on 2022/6/21.
//

#ifndef APP_HTTPWEBCOMPONENT_H
#define APP_HTTPWEBCOMPONENT_H
#include "Http/Client/Http.h"
#include "Http/Common/Config.h"
#include "Http/Common/ContentType.h"
#include "Entity/Component/Component.h"
#include "Server/Component/ITcpComponent.h"


namespace http
{
	class Content;
    class Request;
    class Response;
	class Content;
	class Session;
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
	using IHttpRecordComponent = IRequest<HttpMethodConfig, http::Request, http::Response>;
	class HttpWebComponent final : public Component, public IServerRecord,
								   public IRpc<http::Request, http::Response>, public ITcpListen
    {
    public:
        HttpWebComponent();
    private:
		bool LateAwake() final;
		void OnRecord(json::w::Document& document) final;
	private:
		void StartClose(int id, int) final;
		bool OnListen(tcp::Socket *socket) noexcept final;
		void OnReadHead(http::Request *request, http::Response *response) noexcept final;
		void OnMessage(http::Request * request, http::Response * response) noexcept final;
		void OnApi(const HttpMethodConfig* config, http::Request * request, http::Response * response) noexcept;
		void Invoke(const HttpMethodConfig* config, http::Request * request, http::Response * response) noexcept;
	private:
		bool ReadMessageBody(int id);
		bool SendResponse(int fd, HttpStatus code);
		HttpStatus OnNotFound(http::Request* request, http::Response* response) noexcept;
		HttpStatus AuthToken(const HttpMethodConfig* config, http::Request *request) noexcept;
		HttpStatus CreateHttpData(const HttpMethodConfig* config, http::Request * request) noexcept;
	public:
		bool AddRootDirector(const std::string & dir);
	private:
		std::string mPath;
		http::Config mConfig;
		unsigned int mSuccessCount; //成功次数
		unsigned int mFailureCount; //失败次数
		http::ContentFactory mFactory;
		math::NumberPool<int> mNumPool;
		IHttpRecordComponent * mRecord;
		std::vector<std::string> mRoots;
		class CoroutineComponent * mCoroutine;
		custom::HashMap<std::string, class HttpService *> mHttpServices;
		std::unordered_map<int, std::shared_ptr<http::Session>> mHttpClients;
	};
}


#endif //APP_HTTPWEBCOMPONENT_H
