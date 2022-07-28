//
// Created by 64658 on 2021/8/5.
//
#include"App/App.h"
#include"HttpComponent.h"
#include"Method/HttpServiceMethod.h"
#include"Other/InterfaceConfig.h"
#include"Util/DirectoryHelper.h"
#include"Component/Scene/LoggerComponent.h"
#include"Component/Scene/NetThreadComponent.h"
#include"Network//Http/HttpRequestClient.h"
#include"Script/Extension/Http/LuaHttp.h"
namespace Sentry
{
	bool HttpComponent::LateAwake()
	{
		this->mTaskComponent = this->GetApp()->GetTaskComponent();
		this->mTimeComponent = this->GetApp()->GetTimerComponent();
#ifndef ONLY_MAIN_THREAD
		this->mThreadComponent = this->GetComponent<NetThreadComponent>();
#endif
		return true;
	}

	std::shared_ptr<HttpRequestClient> HttpComponent::CreateClient()
	{
#ifdef ONLY_MAIN_THREAD
		asio::io_context& thread = this->GetApp()->GetThread();
#else
		asio::io_service &thread = this->mThreadComponent->AllocateNetThread();
#endif
		std::shared_ptr<SocketProxy> socketProxy(new SocketProxy(thread));
		return std::make_shared<HttpRequestClient>(socketProxy, this);
	}

	std::shared_ptr<HttpAsyncResponse> HttpComponent::Get(const std::string& url, float second)
	{
        std::shared_ptr<HttpGetRequest> httpGetRequest = HttpGetRequest::Create(url);
        if(httpGetRequest == nullptr)
        {
            LOG_FATAL("parse [" << url << "] error");
            return nullptr;
        }
        std::shared_ptr<HttpTask> httpRpcTask = httpGetRequest->MakeTask(second);
        std::shared_ptr<HttpRequestClient> httpAsyncClient = this->CreateClient();

        this->AddTask(httpRpcTask);
        httpAsyncClient->Request(httpGetRequest);
        return httpRpcTask->Await();
	}

	void HttpComponent::OnLuaRegister(Lua::ClassProxyHelper& luaRegister)
	{
		luaRegister.BeginRegister<HttpComponent>();
		luaRegister.PushExtensionFunction("Get", Lua::Http::Get);
		luaRegister.PushExtensionFunction("Post", Lua::Http::Post);
		luaRegister.PushExtensionFunction("Download", Lua::Http::Download);
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
        std::shared_ptr<HttpTask> httpTask = httpRequest->MakeTask(0);
		std::shared_ptr<HttpRequestClient> requestClient = this->CreateClient();

        this->AddTask(httpTask);
        requestClient->Request(httpRequest, fs);
        std::shared_ptr<HttpAsyncResponse> response = httpTask->Await();

        return response->GetCode() ? XCode::Failure : XCode::Successful;
	}

	std::shared_ptr<HttpAsyncResponse> HttpComponent::Post(const std::string& url, const std::string& data, float second)
	{
        std::shared_ptr<HttpPostRequest> postRequest = HttpPostRequest::Create(url);
        if(postRequest == nullptr)
        {
            return nullptr;
        }
        postRequest->AddBody(data);
        std::shared_ptr<HttpTask> httpTask = postRequest->MakeTask(second);
		std::shared_ptr<HttpRequestClient> httpAsyncClient = this->CreateClient();

        this->AddTask(httpTask);
        httpAsyncClient->Request(postRequest);

        return httpTask->Await();
	}

	void HttpComponent::OnAddTask(RpcTask task)
	{
		long long taskId = task->GetRpcId();
		long long timerId = this->mTimeComponent->DelayCall(
			10, &HttpComponent::OnTimerout, this, taskId);
		this->mTimers.emplace(timerId, taskId);
	}

	void HttpComponent::OnDelTask(long long taskId, RpcTask task)
	{
		auto iter = this->mTimers.find(taskId);
		if(iter != this->mTimers.end())
		{
			this->mTimers.erase(iter);
			LOG_FATAL("http task " << taskId << " time out");
		}
	}

	void HttpComponent::OnTimerout(long long timerId)
	{
		auto iter = this->mTimers.find(timerId);
		if(iter != this->mTimers.end())
		{
			long long id = iter->second;
			this->mTimers.erase(iter);
			this->OnResponse(id, nullptr);
		}
	}
}