//
// Created by 64658 on 2021/8/5.
//
#include"HttpComponent.h"
#include"XCode/XCode.h"
#include"Server/Component/ThreadComponent.h"
#include"Http/Client/RequestClient.h"
#include"Http/Task/HttpTask.h"
#include"Http/Lua/LuaHttp.h"
#include"Entity/Actor/App.h"
#include"Lua/Engine/ModuleClass.h"
#include"Http/Common/HttpRequest.h"
#include"Util/File/DirectoryHelper.h"
#include"Rpc/Component/DispatchComponent.h"

namespace joke
{

	HttpComponent::HttpComponent()
	{
		this->mNetComponent = nullptr;
	}

#ifdef __ENABLE_OPEN_SSL__
	bool HttpComponent::Awake()
	{
		std::unique_ptr<json::r::Value> jsonObj;
		std::unique_ptr<json::r::Value> jsonValue;
		if(!ServerConfig::Inst()->Get("ssl", jsonObj))
		{
			LOG_ERROR("not config ssl field");
			return false;
		}
		if(!jsonObj->Get("client", jsonValue))
		{
			LOG_ERROR("not config client ssl");
			return false;
		}
		Asio::Code code;
		jsonValue->Get("pem", this->mPemPath);
		return code.value() == Asio::OK;
	}
#endif

	bool HttpComponent::LateAwake()
	{
        this->mNetComponent = this->GetComponent<ThreadComponent>();
		return true;
	}

	http::RequestClient * HttpComponent::CreateClient(http::Request * request)
	{
		tcp::Socket* socketProxy = nullptr;
		http::RequestClient* httpClient = nullptr;
#ifdef __ENABLE_OPEN_SSL__
		if(request->IsHttps())
		{
			asio::ssl::context * context = nullptr;
			const std::string & path = request->GetVerifyFile();
			const std::string & fullPath = path.empty() ? this->mPemPath : path;
			auto iter = this->mSslContexts.find(fullPath);
			if(iter == this->mSslContexts.end())
			{
				Asio::Code code;
				context = new asio::ssl::context(asio::ssl::context::sslv23);
				context->load_verify_file(fullPath, code);
				if(code.value() != Asio::OK)
				{
					LOG_ERROR("load ssh : {}", fullPath);
					return nullptr;
				}
				this->mSslContexts.emplace(fullPath, context);
			}
			else
			{
				context = iter->second;
			}
			socketProxy = this->mNetComponent->CreateSocket(*context);
		}
		else
		{
			socketProxy = this->mNetComponent->CreateSocket();
		}
#else
		socketProxy = this->mNetComponent->CreateSocket();
#endif
		httpClient = this->mClientPools.Pop();
		if (httpClient == nullptr)
		{
			httpClient = new http::RequestClient(this);
		}
		httpClient->SetSocket(socketProxy);
		return httpClient;
	}

	http::Response * HttpComponent::Get(const std::string& url, int second)
	{
		std::unique_ptr<http::Request> request = std::make_unique<http::Request>("GET");
		if (!request->SetUrl(url))
		{
			LOG_ERROR("parse {} fail", url);
			return nullptr;
		}
		request->SetTimeout(second);
		http::Response* response = this->Do(std::move(request));
		if (response != nullptr && response->Code() != HttpStatus::OK)
		{
			LOG_ERROR("[GET] {} error={}", url, HttpStatusToString(response->Code()))
		}
		return response;
	}

	void HttpComponent::OnLuaRegister(Lua::ModuleClass &luaRegister)
	{
		luaRegister.AddFunction("Do", Lua::HttpClient::Do);
		luaRegister.AddFunction("Get", Lua::HttpClient::Get);
		luaRegister.AddFunction("Post", Lua::HttpClient::Post);
		luaRegister.AddFunction("Upload", Lua::HttpClient::Upload);
		luaRegister.AddFunction("Download", Lua::HttpClient::Download);

		luaRegister.End("net.http");
	}

	int HttpComponent::Download(const std::string& url, const std::string& path)
	{
		if (!help::dir::IsValidPath(path))
		{
			return XCode::CallArgsError;
		}

		std::unique_ptr<http::Request> request = std::make_unique<http::Request>("GET");
		if (!request->SetUrl(url))
		{
			LOG_ERROR("parse {} fail", url);
			return XCode::Failure;
		}

		HttpRequestTask* httpRpcTask = new HttpRequestTask();
		std::unique_ptr<http::Response> response = std::make_unique<http::Response>();
		{
			if(!response->OpenOrCreateFile(path))
			{
				delete httpRpcTask;
				LOG_ERROR("open or create file : {}", path);
				return XCode::Failure;
			}

			int taskId = 0;
			this->AddTask(taskId, httpRpcTask);
			this->Send(std::move(request), std::move(response), taskId);
		}
		return httpRpcTask->Await()->Code() == HttpStatus::OK;
	}

	void HttpComponent::OnTaskComplete(int key)
	{
		http::RequestClient * httpClient = nullptr;
		if(this->mUseClients.Del(key, httpClient))
		{
			this->mClientPools.Push(httpClient);
		}
	}

	int HttpComponent::Send(std::unique_ptr<http::Request> request, std::function<void(http::Response *)> && cb)
	{
		request->Header().SetKeepAlive(false);
		const http::Url & url = request->GetUrl();
		int taskId = this->mNumPool.BuildNumber();
		std::unique_ptr<http::Response> response = std::make_unique<http::Response>();
		http::RequestClient * httpAsyncClient = this->CreateClient(request.get());
		LOG_DEBUG("connect {} server => {}:{}", url.Protocol(), url.Host(), url.Port());
		{
			this->mUseClients.Add(taskId, httpAsyncClient);
			this->AddTask(taskId, new HttpCallbackTask(cb));
			httpAsyncClient->Do(std::move(request), std::move(response), taskId);
		}
		return XCode::Ok;
	}

	int HttpComponent::Send(std::unique_ptr<http::Request> request, std::unique_ptr<http::Response> response, int& taskId)
	{
		request->Header().SetKeepAlive(false);
		taskId = this->mNumPool.BuildNumber();
		const http::Url & url = request->GetUrl();
		http::RequestClient * httpAsyncClient = this->CreateClient(request.get());
		//LOG_DEBUG("connect {} server => {}:{}", url.Protocol(), url.Host(), url.Port());
		{
			this->mUseClients.Add(taskId, httpAsyncClient);
			httpAsyncClient->Do(std::move(request), std::move(response), taskId);
		}
		return XCode::Ok;
	}

	http::Response * HttpComponent::Do(std::unique_ptr<http::Request> request)
	{
		std::unique_ptr<http::Response> response = std::make_unique<http::Response>();
		{
			int taskId = 0;
			this->Send(std::move(request), std::move(response), taskId);
			return this->AddTask(taskId, new HttpRequestTask())->Await();
		}
	}

	http::Response* HttpComponent::Do(std::unique_ptr<http::Request> request, std::unique_ptr<http::Data> body)
	{
		std::unique_ptr<http::Response> response = std::make_unique<http::Response>();
		{
			int taskId = 0;
			response->SetContent(std::move(body));
			this->Send(std::move(request), std::move(response), taskId);
			return this->AddTask(taskId, new HttpRequestTask())->Await();
		}
	}

	http::Response * HttpComponent::Post(const std::string& url, const std::string& data, int second)
	{
		std::unique_ptr<http::Request> request = std::make_unique<http::Request>("POST");
		if (!request->SetUrl(url))
		{
			return nullptr;
		}
		request->SetTimeout(second);
		request->SetContent(http::Header::JSON, data);
		http::Response * response = this->Do(std::move(request));
		if (response != nullptr && response->Code() != HttpStatus::OK)
		{
			LOG_ERROR("[POST] {} fail:{}", url, HttpStatusToString(response->Code()))
		}
		return response;
	}

	void HttpComponent::OnMessage(http::Request* request, http::Response* response)
	{
		int taskId = 0;
		delete request;
		response->Header().Del("t", taskId);
		this->OnResponse(taskId, response);
	}
}