//
// Created by 64658 on 2021/8/5.
//
#include"HttpComponent.h"
#include"Config/MethodConfig.h"
#include"Component/LogComponent.h"
#include"Component/ThreadComponent.h"
#include"Client/HttpRequestClient.h"
#include"Task/HttpTask.h"
#include"Lua/LuaHttp.h"
namespace Sentry
{
	bool HttpComponent::LateAwake()
	{
		this->mTaskComponent = this->mApp->GetTaskComponent();
        this->mNetComponent = this->GetComponent<ThreadComponent>();
		return true;
	}

	std::shared_ptr<HttpRequestClient> HttpComponent::CreateClient()
	{
        std::shared_ptr<HttpRequestClient> httpClient;
		std::shared_ptr<SocketProxy> socketProxy  = this->mNetComponent->CreateSocket();
        if(!this->mClientPools.empty())
        {
            httpClient = this->mClientPools.front();
            this->mClientPools.pop();
            httpClient->Reset(socketProxy);
            return httpClient;
        }
		return std::make_shared<HttpRequestClient>(socketProxy, this);
	}

	std::shared_ptr<Http::Response> HttpComponent::Get(const std::string& url, float second)
    {
        std::shared_ptr<Http::GetRequest> httpGetRequest(new Http::GetRequest());
        if (!httpGetRequest->SetUrl(url))
        {
            LOG_ERROR("parse " << url << " error");
            return nullptr;
        }

        std::shared_ptr<HttpRequestTask> httpRpcTask(new HttpRequestTask());
        std::shared_ptr<HttpRequestClient> httpAsyncClient = this->CreateClient();

        long long taskId = httpAsyncClient->Do(httpGetRequest);
        std::shared_ptr<Http::Response> response = this->AddTask(taskId, httpRpcTask)->Await();
        if (response->Code() != HttpStatus::OK)
        {
            LOG_ERROR("[GET] " << url << " error = "
                               << HttpStatusToString(response->Code()));
        }
        if (this->mClientPools.size() < 100)
        {
            this->mClientPools.push(httpAsyncClient);
        }
        return response;
    }

	void HttpComponent::OnLuaRegister(Lua::ClassProxyHelper& luaRegister)
	{
        luaRegister.BeginNewTable("Http");
		luaRegister.PushExtensionFunction("Get", Lua::HttpClient::Get);
		luaRegister.PushExtensionFunction("Post", Lua::HttpClient::Post);
		luaRegister.PushExtensionFunction("Download", Lua::HttpClient::Download);
	}

	int HttpComponent::Download(const string& url, const string& path)
    {
        return XCode::Successful;
    }

	std::shared_ptr<Http::Response> HttpComponent::Post(const std::string& url, const std::string& data, float second)
    {
        std::shared_ptr<Http::PostRequest> postRequest(new Http::PostRequest());
        if (postRequest->SetUrl(url))
        {
            return nullptr;
        }
        postRequest->Json(data);
        std::shared_ptr<HttpRequestTask> httpTask(new HttpRequestTask());
        std::shared_ptr<HttpRequestClient> httpAsyncClient = this->CreateClient();

        long long taskId = httpAsyncClient->Do(postRequest);
        std::shared_ptr<Http::Response> response = this->AddTask(taskId, httpTask)->Await();
        if (response->Code() != HttpStatus::OK)
        {
            LOG_ERROR("[POST] " << url << " error = "
                               << HttpStatusToString(response->Code()));
        }
        if (this->mClientPools.size() < 100)
        {
            this->mClientPools.push(httpAsyncClient);
        }
        return response;
    }
}