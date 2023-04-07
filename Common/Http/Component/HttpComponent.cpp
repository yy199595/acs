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
#include"Network/Tcp/Asio.h"
#include"Script/Lua/ClassProxyHelper.h"
#include"Http/Common/HttpRequest.h"
#include"Util/File/DirectoryHelper.h"
//#include"Http/Common/HttpResponse.h"
namespace Tendo
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

	std::shared_ptr<Http::DataResponse> HttpComponent::Get(const std::string& url, bool async, int second)
    {
        std::shared_ptr<Http::GetRequest> request(new Http::GetRequest());
        if (!request->SetUrl(url))
        {
            LOG_ERROR("parse " << url << " error");
            return nullptr;
        }

		if(async)
		{
			request->SetAsync(async);
			request->SetTimeout(second);
			std::shared_ptr<Http::DataResponse> response = this->AsyncRequest(request);
			if (response != nullptr && response->Code() != HttpStatus::OK)
			{
				LOG_ERROR("[GET] " << url << " error = "
								   << HttpStatusToString(response->Code()));
			}
			return response;
		}
		else
		{
			std::shared_ptr<Http::IResponse> response
				= std::make_shared<Http::DataResponse>();
			if(!this->Send(request, response))
			{
				return nullptr;
			}
			if(response->Code() != HttpStatus::OK)
			{
				LOG_ERROR("[GET] " << url << " error = "
								   << HttpStatusToString(response->Code()));
			}
			return std::static_pointer_cast<Http::DataResponse>(response);
		}
    }

	void HttpComponent::OnLuaRegister(Lua::ClassProxyHelper& luaRegister)
	{
        luaRegister.BeginNewTable("Http");
		luaRegister.PushExtensionFunction("Get", Lua::HttpClient::Get);
		luaRegister.PushExtensionFunction("Post", Lua::HttpClient::Post);
		luaRegister.PushExtensionFunction("Download", Lua::HttpClient::Download);
	}

	bool HttpComponent::Download(const string& url, const string& path, bool async)
    {
		if (!Helper::Directory::IsValidPath(path))
		{
			return XCode::CallArgsError;
		}

		std::shared_ptr<Http::GetRequest> request(new Http::GetRequest());
		if (!request->SetUrl(url))
		{
			LOG_ERROR("parse " << url << " error");
			return XCode::Failure;
		}

		if(async)
		{
			request->SetAsync(async);
			std::shared_ptr<HttpRequestTask> httpRpcTask = std::make_shared<HttpRequestTask>();
			std::shared_ptr<Http::IResponse> response = std::make_shared<Http::FileResponse>(path);
			{
				int taskId = 0;
				this->AddTask(taskId, httpRpcTask);
				this->Send(request, response, taskId);
			}
			return httpRpcTask->Await()->Code() == HttpStatus::OK;
		}
		else
		{
			std::shared_ptr<Http::IResponse> response
				= std::make_shared<Http::FileResponse>(path);
			if(!this->Send(request, response))
			{
				return false;
			}
			return response->Code() == HttpStatus::OK;
		}
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

	bool HttpComponent::Send(const std::shared_ptr<Http::Request>& request,
		std::shared_ptr<Http::IResponse> & response, int& taskId)
	{
		if (!request->Async())
		{
			return false;
		}
		taskId = this->mNumberPool.Pop();
		std::shared_ptr<HttpRequestClient> httpAsyncClient = this->CreateClient();
		{
			httpAsyncClient->Do(request, response, taskId);
			this->mUseClients.emplace(taskId, httpAsyncClient);
		}
		return true;
	}

	std::shared_ptr<Http::DataResponse> HttpComponent::AsyncRequest(const std::shared_ptr<Http::Request>& request)
	{
		std::shared_ptr<HttpRequestClient> httpAsyncClient = this->CreateClient();
		std::shared_ptr<HttpRequestTask> httpRpcTask = std::make_shared<HttpRequestTask>();
		std::shared_ptr<Http::IResponse> response = std::make_shared<Http::DataResponse>();
		{
			int taskId = 0;
			this->AddTask(taskId, httpRpcTask);
			this->Send(request, response, taskId);
		}
		return std::static_pointer_cast<Http::DataResponse>(httpRpcTask->Await());
	}

	std::shared_ptr<Http::DataResponse> HttpComponent::Post(const std::string& url,
			const std::string& data, bool async, int second)
    {
        std::shared_ptr<Http::PostRequest> request(new Http::PostRequest());
        if (request->SetUrl(url))
        {
            return nullptr;
        }
		request->Json(data);
		request->SetAsync(async);
		request->SetTimeout(second);
        std::shared_ptr<Http::DataResponse> response = this->AsyncRequest(request);
        if (response != nullptr && response->Code() != HttpStatus::OK)
        {
            LOG_ERROR("[POST] " << url << " error = "
                               << HttpStatusToString(response->Code()));
        }
        return response;
    }

	bool HttpComponent::Send(const std::shared_ptr<Http::Request>& request, std::shared_ptr<Http::IResponse>& response)
	{
		assert(request->Async());
		try
		{
			asio::io_context io;
			asio::ip::tcp::resolver resolver(io);
			asio::ip::tcp::resolver::results_type endpoints =
					resolver.resolve(request->Host(), request->Port());

			asio::ip::tcp::socket socket(io);
			asio::connect(socket, endpoints);

			asio::streambuf requestBuf;
			std::ostream requestStream(&requestBuf);
			while (request->Serialize(requestStream) <= 0)
			{
				asio::write(socket, requestBuf);
			}
			asio::streambuf responseBuf;
			std::istream responseStream(&requestBuf);
			asio::read_until(socket, responseBuf, "\r\n");
		ON_READ_HANDLE:
			int num = response->OnRead(responseStream);
			switch (num)
			{
				case HTTP_READ_LINE:
					asio::read_until(socket, responseBuf, "\r\n");
					goto ON_READ_HANDLE;
					break;
				case HTTP_READ_SOME:
					asio::read(socket, responseBuf, asio::transfer_at_least(1));
					goto ON_READ_HANDLE;
					break;
				case HTTP_READ_ERROR:
					return false;
				case HTTP_READ_COMPLETE:
					return true;
				default:
					asio::async_read(socket, responseBuf, asio::transfer_exactly(num));
					goto ON_READ_HANDLE;
					break;
			}
		}
		catch(asio::system_error & code)
		{
			return false;
		}
	}
}