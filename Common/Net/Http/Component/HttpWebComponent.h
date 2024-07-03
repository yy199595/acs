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
	class Data;
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

namespace joke
{
    class HttpMethodConfig;
    class HttpWebComponent final : public HttpListenComponent, public IServerRecord
    {
    public:
        HttpWebComponent();
    private:
		bool ReadHttpConfig();
		bool LateAwake() final;
		bool OnListenOk(const char * name) final;
		void OnRecord(json::w::Document &document) final;
		void OnApi(const HttpMethodConfig* config, http::Request * request, http::Response * response);
		void Invoke(const HttpMethodConfig* config, http::Request * request, http::Response * response);
	private:
		void OnNotFound(http::Request * request, http::Response * response);
		void OnReadHead(http::Request *request, http::Response *response) final;
		void OnMessage(http::Request * request, http::Response * response) final;
		HttpStatus AuthToken(const HttpMethodConfig* config, http::Request *request);
		HttpStatus CreateHttpData(const HttpMethodConfig* config, http::Request * request);
	public:
		bool AddRootDirector(const std::string & dir);
		const http::Config & GetConfig() const { return this->mConfig; }
	private:
		std::string mKey;
		http::Config mConfig;
		http::ContentFactory mFactory;
		std::vector<std::string> mRoots;
		class CoroutineComponent * mCorComponent;
		std::unordered_map<std::string, std::string> mDefaultHeader;
		custom::HashMap<std::string, class HttpService *> mHttpServices;
	};
}


#endif //APP_HTTPWEBCOMPONENT_H
