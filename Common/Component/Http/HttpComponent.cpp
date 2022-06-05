//
// Created by 64658 on 2021/8/5.
//
#include"App/App.h"
#include"Thread/TaskThread.h"
#include"HttpComponent.h"
#include"Method/HttpServiceMethod.h"
#include"Other/InterfaceConfig.h"
#include"Util/StringHelper.h"
#include"Util/DirectoryHelper.h"
#include"Component/Scene/LoggerComponent.h"
#include"Component/Scene/NetThreadComponent.h"
#include"Network//Http/HttpRequestClient.h"
#include"Network/Http/HttpHandlerClient.h"
#include"Script/Extension/Http/LuaHttp.h"
#include"Component/HttpService/LocalHttpService.h"
namespace Sentry
{

	void HttpComponent::Awake()
	{

	}

	bool HttpComponent::LateAwake()
	{
		this->mTaskComponent = this->GetApp()->GetTaskComponent();
		this->mTimeComponent = this->GetApp()->GetTimerComponent();
#ifndef ONLY_MAIN_THREAD
		this->mThreadComponent = this->GetComponent<NetThreadComponent>();
#endif
		std::vector<Component *> components;
		this->GetApp()->GetComponents(components);
		for(Component * component : components)
		{
			LocalHttpService * localHttpService = component->Cast<LocalHttpService>();
			if(localHttpService != nullptr)
			{
				std::vector<const HttpInterfaceConfig *> httpInterConfigs;
				localHttpService->GetServiceConfig().GetConfigs(httpInterConfigs);
				for(const HttpInterfaceConfig * httpInterfaceConfig : httpInterConfigs)
				{
					this->mHttpConfigs.emplace(httpInterfaceConfig->Path, httpInterfaceConfig);
				}
			}
		}

		return true;
	}

	void HttpComponent::OnListen(std::shared_ptr<SocketProxy> socket)
	{
		const std::string & address = socket->GetAddress();

		std::shared_ptr<HttpHandlerClient> handlerClient =
			std::make_shared<HttpHandlerClient>(this, socket);

		handlerClient->StartReceive();
		this->mHttpClients.insert(handlerClient);
	}

	void HttpComponent::HandlerHttpData(std::shared_ptr<HttpHandlerClient> httpClient)
	{
		std::shared_ptr<HttpHandlerRequest> request = httpClient->Request();
		std::shared_ptr<HttpHandlerResponse> response = httpClient->Response();

		const std::string& path = request->GetPath();
		auto iter = this->mHttpConfigs.find(path);
		if(iter == this->mHttpConfigs.end())
		{
			this->ClosetHttpClient(httpClient);
			LOG_ERROR("not find http config : " << path);
			return;
		}
		const HttpInterfaceConfig* httpConfig = iter->second;
		if(httpConfig->Type != request->GetMethod())
		{
			this->ClosetHttpClient(httpClient);
			return;
		}
		LocalHttpService* httpService = this->GetComponent<LocalHttpService>(httpConfig->Service);
		if(httpService == nullptr || !httpService->IsStartService())
		{
			this->ClosetHttpClient(httpClient);
			return;
		}
		if(!httpConfig->IsAsync)
		{
			httpService->Invoke(httpConfig->Method, request, response);
			httpClient->StartWriter();
		}
		this->mTaskComponent->Start([httpService, httpClient, httpConfig, request, response]()
		{
			httpService->Invoke(httpConfig->Method, request, response);
			httpClient->StartWriter();
		});
	}

	std::shared_ptr<HttpRequestClient> HttpComponent::CreateClient()
	{
#ifdef ONLY_MAIN_THREAD
		IAsioThread& thread = this->GetApp()->GetTaskScheduler();
#else
		IAsioThread &thread = this->mThreadComponent->AllocateNetThread();
#endif
		std::shared_ptr<SocketProxy> socketProxy(new SocketProxy(thread));
		return std::make_shared<HttpRequestClient>(socketProxy);
	}

	std::shared_ptr<HttpAsyncResponse> HttpComponent::Get(const std::string& url, float second)
	{
		long long timerId = this->mTimeComponent->DelayCall(second, [this, url]()
		{
			LOG_ERROR("get " << url << " timeout");
		});
		std::shared_ptr<HttpRequestClient> httpAsyncClient = this->CreateClient();
		std::shared_ptr<HttpAsyncResponse> httpResponse = httpAsyncClient->Get(url);
		this->mTimeComponent->CancelTimer(timerId);
		if(httpResponse == nullptr)
		{
			LOG_ERROR(url << " net work error");
			return nullptr;
		}
		if(httpResponse->GetHttpCode() != HttpStatus::OK)
		{
			const char * err = HttpStatusToString(httpResponse->GetHttpCode());
			LOG_FATAL("http error = " << err);
			return nullptr;
		}
		return httpResponse;
	}

	void HttpComponent::OnLuaRegister(Lua::ClassProxyHelper& luaRegister)
	{
		luaRegister.BeginRegister<HttpComponent>();
		luaRegister.PushExtensionFunction("Get", Lua::Http::Get);
		luaRegister.PushExtensionFunction("Post", Lua::Http::Post);
		luaRegister.PushExtensionFunction("Download", Lua::Http::Download);

		Lua::ClassProxyHelper classProxyHelper = luaRegister.Clone("HttpHandlerResponse");
		classProxyHelper.BeginRegister<HttpHandlerResponse>();
		classProxyHelper.PushMemberFunction("AddHead", &HttpHandlerResponse::AddHead);
		classProxyHelper.PushMemberFunction("WriteString", &HttpHandlerResponse::WriteString);
	}

	XCode HttpComponent::Download(const string& url, const string& path)
	{
		if(!Helper::Directory::MakeDir(path))
		{
			return XCode::Failure;
		}

		std::fstream * fs = new std::fstream();
		fs->open(path, std::ios::binary | std::ios::in | std::ios::out);
		if(!fs->is_open())
		{
			delete fs;
			return XCode::Failure;
		}
		std::shared_ptr<HttpGetRequest> httpRequest = HttpGetRequest::Create(url);
		if(httpRequest == nullptr)
		{
			return XCode::HttpUrlParseError;
		}
		std::shared_ptr<HttpRequestClient> requestClient = this->CreateClient();
		if(requestClient->Request(httpRequest, fs) != nullptr)
		{
			return XCode::Successful;
		}
		return XCode::NetWorkError;
	}

	std::shared_ptr<HttpAsyncResponse> HttpComponent::Post(const std::string& url, const std::string& data, float second)
	{
		long long timerId = this->mTimeComponent->DelayCall(second, [this, url]()
		{
			LOG_ERROR("post " << url << " timeout");
		});
		std::shared_ptr<HttpRequestClient> httpAsyncClient = this->CreateClient();
		std::shared_ptr<HttpAsyncResponse> httpAsyncResponse = httpAsyncClient->Post(url, data);
		LOG_WARN("post " << url << " successful request = " << data);
		this->mTimeComponent->CancelTimer(timerId);
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
	void HttpComponent::ClosetHttpClient(std::shared_ptr<HttpHandlerClient> httpClient)
	{
		auto iter = this->mHttpClients.find(httpClient);
		if(iter != this->mHttpClients.end())
		{
			this->mHttpClients.erase(iter);
			LOG_DEBUG("remove http address : " << httpClient->GetAddress());
		}
	}

}