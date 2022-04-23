
#pragma once
#include"Json/JsonWriter.h"
#include"Component/Component.h"
namespace Sentry
{
    class HttpConfig
    {
    public:
        std::string Url;
        std::string Type;
		std::string Content;
		std::string Component;
        std::string MethodName;
    };
}
namespace Sentry
{
	class HttpRespSession;
	class HttpServiceMethod;
	class HttpHandlerClient;
	class HttpAsyncResponse;
	class HttpHandlerRequest;
	class HttpComponent : public Component, public ISocketListen
	{
	 public:
		HttpComponent() = default;
		~HttpComponent() final = default;
	 public:
		void Awake() final;
		bool LateAwake() final;
	 public:
		std::shared_ptr<HttpAsyncResponse> Get(const std::string& url, int timeout = 5);

		std::shared_ptr<HttpAsyncResponse> Post(const std::string& url, const std::string& data, int timeout = 5);

		std::shared_ptr<HttpAsyncResponse> Post(const std::string& url, Json::Writer& jsonWriter, int timeout = 5);

	 public:
		void OnListen(std::shared_ptr<SocketProxy> socket) final;
	 private:
		const HttpConfig* GetHttpConfig(const std::string& url);
		void HandlerHttpData(std::shared_ptr<HttpHandlerClient> httpClient);
		XCode Invoke(const HttpConfig* config, std::shared_ptr<HttpHandlerRequest> content, std::shared_ptr<Json::Writer> response);
	 private:
		class TaskComponent* mCorComponent;
		class ThreadPoolComponent* mThreadComponent;
		std::unordered_map<std::string, HttpConfig*> mHttpConfigMap;
	};
}