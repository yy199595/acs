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
#include"Network/Listener/TcpServerComponent.h"
namespace Sentry
{
	bool HttpComponent::LateAwake()
	{
		this->mTaskComponent = this->GetApp()->GetTaskComponent();
		this->mTimeComponent = this->GetApp()->GetTimerComponent();
        this->mTcpComponent = this->GetComponent<TcpServerComponent>();
		return true;
	}

	std::shared_ptr<HttpRequestClient> HttpComponent::CreateClient()
	{
        std::shared_ptr<HttpRequestClient> httpClient;
		std::shared_ptr<SocketProxy> socketProxy  = this->mTcpComponent->CreateSocket();
        if(!this->mClientPools.empty())
        {
            httpClient = this->mClientPools.front();
            this->mClientPools.pop();
            assert(httpClient->Reset(socketProxy));
            return httpClient;
        }
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
        std::shared_ptr<HttpAsyncResponse> response = httpRpcTask->Await();
        if(this->mClientPools.size() < 100)
        {
            this->mClientPools.push(httpAsyncClient);
            this->mTcpComponent->DeleteSocket(httpAsyncClient->MoveSocket());
        }
        return response;
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
        if(this->mClientPools.size() < 100)
        {
            this->mClientPools.push(requestClient);
            this->mTcpComponent->DeleteSocket(requestClient->MoveSocket());
        }

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
        std::shared_ptr<HttpAsyncResponse> response = httpTask->Await();
        if(this->mClientPools.size() < 100)
        {
            this->mClientPools.push(httpAsyncClient);
            this->mTcpComponent->DeleteSocket(httpAsyncClient->MoveSocket());
        }
        return response;
	}

	void HttpComponent::OnAddTask(RpcTask task)
	{
		long long taskId = task->GetRpcId();
		long long timerId = this->mTimeComponent->DelayCall(
			10, &HttpComponent::OnTimeout, this, taskId);
		this->mTimers.emplace(taskId, timerId);
	}

	void HttpComponent::OnDelTask(long long taskId, RpcTask task)
	{
		auto iter = this->mTimers.find(taskId);
		if(iter != this->mTimers.end())
		{
			long long id = iter->second;
			this->mTimers.erase(iter);
			this->mTimeComponent->CancelTimer(id);
		}
	}
}