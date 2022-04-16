//
// Created by 64658 on 2021/8/5.
//
#include"App/App.h"
#include"Util/FileHelper.h"
#include"Thread/TaskThread.h"
#include"HttpComponent.h"
#include"Method/HttpServiceMethod.h"
#include"Other/ProtoConfig.h"
#include"Other/ElapsedTimer.h"
#include"Component/Scene/LoggerComponent.h"
#include"Component/Scene/ThreadPoolComponent.h"
#include"Network//Http/HttpRequestClient.h"
#include"Network/Http/HttpHandlerClient.h"
#include"Component/HttpService/HttpService.h"
namespace Sentry
{

	void HttpComponent::Awake()
	{
		std::string path;
		this->mCorComponent = nullptr;
		rapidjson::Document jsonDocument;
		const ServerConfig& config = App::Get()->GetConfig();
		if (config.GetMember("path", "http", path))
		{
			LOG_CHECK_RET(Helper::File::ReadJsonFile(path, jsonDocument));
			for (auto iter = jsonDocument.MemberBegin(); iter != jsonDocument.MemberEnd(); iter++)
			{
				HttpConfig* httpConfig = new HttpConfig();
				httpConfig->Url = iter->name.GetString();
				const rapidjson::Value& jsonValue = iter->value;
				httpConfig->Type = jsonValue["Type"].GetString();
				httpConfig->MethodName = jsonValue["Method"].GetString();
				httpConfig->Component = jsonValue["Component"].GetString();
				if (jsonValue.HasMember("Content"))
				{
					httpConfig->Content = jsonValue["Content"].GetString();
				}
				this->mHttpConfigMap.emplace(httpConfig->Url, httpConfig);
			}
		}
	}

	bool HttpComponent::LateAwake()
	{
		this->mCorComponent = App::Get()->GetTaskComponent();
		this->mThreadComponent = this->GetComponent<ThreadPoolComponent>();

		std::string url1 = "http://v.juhe.cn/telecode/to_telecodes.php";
		this->mCorComponent->Start([this, url1]()
		{

		});
		return true;
	}

	void HttpComponent::OnListen(std::shared_ptr<SocketProxy> socket)
	{
		std::shared_ptr<HttpHandlerClient> handlerClient(new HttpHandlerClient(socket));
		this->mCorComponent->Start(&HttpComponent::HandlerHttpData, this, handlerClient);
	}

	const HttpConfig* HttpComponent::GetHttpConfig(const std::string& url)
	{
		auto iter = this->mHttpConfigMap.find(url);
		return iter != this->mHttpConfigMap.end() ? iter->second : nullptr;
	}

	void HttpComponent::HandlerHttpData(std::shared_ptr<HttpHandlerClient> httpClient)
	{
		std::shared_ptr<HttpHandlerRequest> httpRequestData = httpClient->ReadHandlerContent();
		LOG_CHECK_RET(httpRequestData);

		std::string host;
		const std::string& url = httpRequestData->GetUrl();
		const HttpConfig* httpConfig = this->GetHttpConfig(url);
		httpRequestData->GetHeadContent("host", host);
#ifdef __DEBUG__
		ElapsedTimer elapsedTimer;
		LOG_INFO("http://" << host << url);
		std::shared_ptr<Json::Writer> jsonResponse = this->Invoke(httpConfig, httpRequestData);
#endif
		if (jsonResponse != nullptr)
		{
			httpClient->Response(HttpStatus::OK, *jsonResponse);
		}
#ifdef __DEBUG__
		LOG_INFO("==== http request handler ====");
		LOG_INFO("url = " << httpRequestData->GetUrl());
		LOG_INFO("type = " << httpRequestData->GetMethod());
		LOG_INFO("time = " << elapsedTimer.GetMs() << " ms");
		if (!httpRequestData->GetContent().empty())
		{
			LOG_INFO("request = " << httpRequestData->GetContent());
		}
		LOG_INFO("response = " << jsonResponse->ToJsonString());
#endif
	}

	std::shared_ptr<Json::Writer> HttpComponent::Invoke(const HttpConfig* httpConfig, std::shared_ptr<HttpHandlerRequest> content)
	{
		std::shared_ptr<Json::Writer> jsonWriter(new Json::Writer());
		if (httpConfig == nullptr)
		{
			jsonWriter->AddMember("code", XCode::CallServiceNotFound);
			jsonWriter->AddMember("error", fmt::format("not find url : {0}", content->GetUrl()));
			return jsonWriter;
		}

		if (httpConfig->Type != content->GetMethod())
		{
			jsonWriter->AddMember("code", XCode::HttpMethodNotFound);
			jsonWriter->AddMember("error", fmt::format("user {0} call", httpConfig->Type));
			return jsonWriter;
		}

		std::shared_ptr<Json::Reader> jsonReader(new Json::Reader());
		if (!jsonReader->ParseJson(content->GetContent()))
		{
			jsonWriter->AddMember("code", XCode::ParseJsonFailure);
			jsonWriter->AddMember("error", "json parse error");
			return jsonWriter;
		}
		HttpService* httpService = this->GetComponent<HttpService>(httpConfig->Component);
		if (httpService == nullptr)
		{
			jsonWriter->AddMember("code", XCode::CallServiceNotFound);
			jsonWriter->AddMember("error", "not find handler component");
			return jsonWriter;
		}
		const std::string& method = httpConfig->MethodName;
		return httpService->Invoke(method, jsonReader);
	}

	std::shared_ptr<HttpAsyncResponse> HttpComponent::Get(const std::string& url, int timeout)
	{
#ifdef ONLY_MAIN_THREAD
		IAsioThread& thread = this->GetApp()->GetTaskScheduler();
#else
		IAsioThread &thread = this->mThreadComponent->AllocateNetThread();
#endif
		std::shared_ptr<SocketProxy> socketProxy(new SocketProxy(thread, "HttpRequest"));
		std::shared_ptr<HttpRequestClient> httpAsyncClient(new HttpRequestClient(socketProxy));
		return httpAsyncClient->Get(url);
	}

	std::shared_ptr<HttpAsyncResponse>
	HttpComponent::Post(const std::string& url, const std::string& data, int timeout)
	{
#ifdef ONLY_MAIN_THREAD
		IAsioThread& thread = this->GetApp()->GetTaskScheduler();
#else
		IAsioThread &thread = this->mThreadComponent->AllocateNetThread();
#endif
		std::shared_ptr<SocketProxy> socketProxy(new SocketProxy(thread, "HttpRequest"));
		std::shared_ptr<HttpRequestClient> httpAsyncClient(new HttpRequestClient(socketProxy));
		return httpAsyncClient->Post(url, data);
	}

	std::shared_ptr<HttpAsyncResponse>
	HttpComponent::Post(const std::string& url, Json::Writer& jsonWriter, int timeout)
	{
		return this->Post(url, jsonWriter.ToJsonString());
	}
}