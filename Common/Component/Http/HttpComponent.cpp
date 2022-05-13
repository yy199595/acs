//
// Created by 64658 on 2021/8/5.
//
#include"App/App.h"
#include"Util/FileHelper.h"
#include"Thread/TaskThread.h"
#include"HttpComponent.h"
#include"Method/HttpServiceMethod.h"
#include"Other/InterfaceConfig.h"
#include"Other/ElapsedTimer.h"
#include"Util/StringHelper.h"
#include"Component/Scene/LoggerComponent.h"
#include"Component/Scene/ThreadPoolComponent.h"
#include"Network//Http/HttpRequestClient.h"
#include"Network/Http/HttpHandlerClient.h"
#include"Component/HttpService/HttpService.h"
namespace Sentry
{

	void HttpComponent::Awake()
	{

	}

	bool HttpComponent::LateAwake()
	{
		this->mCorComponent = App::Get()->GetTaskComponent();
		this->mThreadComponent = this->GetComponent<ThreadPoolComponent>();
		return true;
	}

	void HttpComponent::OnListen(std::shared_ptr<SocketProxy> socket)
	{
		std::shared_ptr<HttpHandlerClient> handlerClient(new HttpHandlerClient(socket));
		this->mCorComponent->Start(&HttpComponent::HandlerHttpData, this, handlerClient);
	}

	void HttpComponent::HandlerHttpData(std::shared_ptr<HttpHandlerClient> httpClient)
	{
		std::shared_ptr<Json::Writer> jsonResponse(new Json::Writer());
		std::shared_ptr<HttpHandlerRequest> httpRequestData = httpClient->ReadHandlerContent();
		LOG_CHECK_RET(httpRequestData);

		std::string host;
		const std::string& url = httpRequestData->GetPath();
		const ServiceConfig & serviceConfig = this->GetApp()->GetServiceConfig();
		const HttpInterfaceConfig* httpConfig =serviceConfig.GetHttpIterfaceConfig(url);
#ifdef __DEBUG__
		ElapsedTimer elapsedTimer;
#endif
		XCode code = this->Invoke(httpConfig, httpRequestData, jsonResponse);

		jsonResponse->AddMember("code", code);
		httpClient->Response(HttpStatus::OK, *jsonResponse);
#ifdef __DEBUG__
		LOG_INFO("==== http request handler ====");
		LOG_INFO("url = " << httpRequestData->GetPath());
		LOG_INFO("type = " << httpRequestData->GetMethod());
		LOG_INFO("time = " << elapsedTimer.GetMs() << " ms");
		if (!httpRequestData->GetContent().empty())
		{
			LOG_INFO("request = " << httpRequestData->GetContent());
		}
		LOG_INFO("response = " << jsonResponse->ToJsonString());
#endif
	}

	XCode HttpComponent::Invoke(const HttpInterfaceConfig* httpConfig, std::shared_ptr<HttpHandlerRequest> content,
		std::shared_ptr<Json::Writer> response)
	{
		std::shared_ptr<Json::Writer> jsonWriter(new Json::Writer());
		if (httpConfig == nullptr)
		{
			jsonWriter->AddMember("error", fmt::format("not find url : {0}", content->GetPath()));
			return XCode::CallServiceNotFound;
		}

		if (httpConfig->Type != content->GetMethod())
		{
			jsonWriter->AddMember("error", fmt::format("user {0} call", httpConfig->Type));
			return XCode::HttpMethodNotFound;
		}

		std::shared_ptr<Json::Reader> jsonReader(new Json::Reader());
		if (!jsonReader->ParseJson(content->GetContent()))
		{
			jsonWriter->AddMember("error", "json parse error");
			return XCode::ParseJsonFailure;
		}
		std::string ip;
		unsigned short port = 0;
		Helper::String::ParseIpAddress(content->GetAddress(), ip, port);
		jsonReader->AddMember("ip", rapidjson::StringRef(ip.c_str()), jsonReader->GetAllocator());
		HttpService* httpService = this->GetComponent<HttpService>(httpConfig->Service);
		if (httpService == nullptr)
		{
			jsonWriter->AddMember("error", "not find handler component");
			return XCode::CallServiceNotFound;
		}
		const std::string& method = httpConfig->Method;

		try
		{
			response->StartObject("data");
			XCode code = httpService->Invoke(method, jsonReader, response);
			response->EndObject();
			return code;
		}
		catch(std::logic_error & logic_error)
		{
			response->EndObject();
			response->AddMember("error", logic_error.what());
			return XCode::ThrowError;
		}
	}

	std::shared_ptr<HttpAsyncResponse> HttpComponent::Get(const std::string& url, int timeout)
	{
#ifdef ONLY_MAIN_THREAD
		IAsioThread& thread = this->GetApp()->GetTaskScheduler();
#else
		IAsioThread &thread = this->mThreadComponent->AllocateNetThread();
#endif
		std::shared_ptr<SocketProxy> socketProxy(new SocketProxy(thread));
		std::shared_ptr<HttpRequestClient> httpAsyncClient(new HttpRequestClient(socketProxy));
		return httpAsyncClient->Get(url);
	}

	std::shared_ptr<HttpAsyncResponse> HttpComponent::Post(const std::string& url, const std::string& data, int timeout)
	{
#ifdef ONLY_MAIN_THREAD
		IAsioThread& thread = this->GetApp()->GetTaskScheduler();
#else
		IAsioThread &thread = this->mThreadComponent->AllocateNetThread();
#endif
		std::shared_ptr<SocketProxy> socketProxy(new SocketProxy(thread));
		std::shared_ptr<HttpRequestClient> httpAsyncClient(new HttpRequestClient(socketProxy));
		std::shared_ptr<HttpAsyncResponse> httpAsyncResponse = httpAsyncClient->Post(url, data);
		if(httpAsyncResponse == nullptr)
		{
			LOG_FATAL(url << " net work error");
			return nullptr;
		}
		if(httpAsyncResponse->GetHttpCode() != HttpStatus::OK)
		{
			const char * err = HttpStatusToString(httpAsyncResponse->GetHttpCode());
			LOG_FATAL("http error = " << err);
			return nullptr;
		}
		return httpAsyncResponse;
	}
}