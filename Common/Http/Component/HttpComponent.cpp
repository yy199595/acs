//
// Created by 64658 on 2021/8/5.
//
#include"HttpComponent.h"

#include"Server/Config/MethodConfig.h"
#include"Log/Component/LogComponent.h"
#include"Server/Component/ThreadComponent.h"
#include"Http/Client/HttpRequestClient.h"
#include"Http/Task/HttpTask.h"
#include"Http/Lua/LuaHttp.h"
#include"Entity/App/App.h"
#include"Script/Lua/ClassProxyHelper.h"
#include"Http/Common/HttpRequest.h"
#include"Util/File/DirectoryHelper.h"
//#include"Http/Common/HttpResponse.h"
namespace Sentry
{
	HttpComponent::HttpComponent()
	{
		this->mNetComponent = nullptr;
	}

	bool HttpComponent::LateAwake()
	{
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

	std::shared_ptr<Http::DataResponse> HttpComponent::Get(const std::string& url, float second)
    {
        std::shared_ptr<Http::GetRequest> request(new Http::GetRequest());
        if (!request->SetUrl(url))
        {
            LOG_ERROR("parse " << url << " error");
            return nullptr;
        }
		std::shared_ptr<Http::DataResponse> response = this->Request(request);
        if (response != nullptr && response->Code() != HttpStatus::OK)
        {
            LOG_ERROR("[GET] " << url << " error = "
                               << HttpStatusToString(response->Code()));
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

	bool HttpComponent::Download(const string& url, const string& path)
    {
		std::shared_ptr<Http::GetRequest> request(new Http::GetRequest());
		if (!request->SetUrl(url))
		{
			LOG_ERROR("parse " << url << " error");
			return XCode::Failure;
		}
		if (!Helper::Directory::IsValidPath(path))
		{
			return XCode::CallArgsError;
		}
	
		std::shared_ptr<HttpRequestTask> httpRpcTask = std::make_shared<HttpRequestTask>();
		std::shared_ptr<Http::FileResponse> response = std::make_shared<Http::FileResponse>(path);
		{
			int taskId = 0;
			this->AddTask(taskId, httpRpcTask);
			this->Send(request, response, taskId);
		}
		return httpRpcTask->Await()->Code() == HttpStatus::OK;
    }

	void HttpComponent::OnTaskComplete(int key)
	{
		this->mNumberPool.Push(key);
		auto iter = this->mUseClients.find(key);
		if(iter != this->mUseClients.end())
		{
			if(this->mClientPools.size() <= 100)
			{
				this->mClientPools.push(iter->second);
			}
			this->mUseClients.erase(key);
		}
	}

	void HttpComponent::Send(const std::shared_ptr<Http::Request>& request, 
		std::shared_ptr<Http::IResponse> response, int& taskId)
	{
		taskId = this->mNumberPool.Pop();
		std::shared_ptr<HttpRequestClient> httpAsyncClient = this->CreateClient();
		{
			
			httpAsyncClient->Do(request, response, taskId);
			this->mUseClients.emplace(taskId, httpAsyncClient);
		}
	}

	std::shared_ptr<Http::DataResponse> HttpComponent::Request(const std::shared_ptr<Http::Request>& request)
	{	
		std::shared_ptr<HttpRequestClient> httpAsyncClient = this->CreateClient();
		std::shared_ptr<HttpRequestTask> httpRpcTask = std::make_shared<HttpRequestTask>();
		std::shared_ptr<Http::DataResponse> response = std::make_shared<Http::DataResponse>();
		{
			int taskId = 0;
			this->AddTask(taskId, httpRpcTask);
			this->Send(request, response, taskId);
		}
		return std::static_pointer_cast<Http::DataResponse>(httpRpcTask->Await());
	}

	std::shared_ptr<Http::DataResponse> HttpComponent::Post(const std::string& url, const std::string& data, float second)
    {
        std::shared_ptr<Http::PostRequest> request(new Http::PostRequest());
        if (request->SetUrl(url))
        {
            return nullptr;
        }
		request->Json(data);
        std::shared_ptr<Http::DataResponse> response = this->Request(request);
        if (response != nullptr && response->Code() != HttpStatus::OK)
        {
            LOG_ERROR("[POST] " << url << " error = "
                               << HttpStatusToString(response->Code()));
        }
        return response;
    }
}