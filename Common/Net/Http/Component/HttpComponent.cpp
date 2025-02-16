//
// Created by 64658 on 2021/8/5.
//
#include"HttpComponent.h"
#include"XCode/XCode.h"
#include"Server/Component/ThreadComponent.h"
#include"Http/Client/Client.h"
#include"Http/Task/HttpTask.h"
#include"Entity/Actor/App.h"
#include"Lua/Engine/ModuleClass.h"
#include"Http/Common/HttpRequest.h"
#include"Util/File/DirectoryHelper.h"

#include "Lua/Lib/Lib.h"
namespace acs
{

	HttpComponent::HttpComponent()
	{
		this->mNetComponent = nullptr;
	}


	bool HttpComponent::Awake()
	{
		LuaCCModuleRegister::Add([](Lua::CCModule & ccModule) {
			ccModule.Open("net.http", lua::lib::luaopen_lhttp);
		});
#ifdef __ENABLE_OPEN_SSL__
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
#else
		return true;
#endif
	}

	bool HttpComponent::LateAwake()
	{
        this->mNetComponent = this->GetComponent<ThreadComponent>();
		return true;
	}

	std::shared_ptr<http::Client> HttpComponent::CreateClient(http::Request * request)
	{
		tcp::Socket* socketProxy = nullptr;
#ifdef __ENABLE_OPEN_SSL__
		if(request->IsHttps())
		{
			const std::string & path = request->GetVerifyFile();
			const std::string & fullPath = path.empty() ? this->mPemPath : path;
			auto iter = this->mSslContexts.find(fullPath);
			if(iter == this->mSslContexts.end())
			{
				Asio::Code code;
				//context->set_options(asio::ssl::context::default_workarounds | asio::ssl::context::no_sslv2 |asio::ssl::context::single_dh_use);
				{
					std::unique_ptr<asio::ssl::context> context = std::make_unique<asio::ssl::context>(asio::ssl::context::sslv23);
					//context->set_verify_mode(asio::ssl::verify_none);
					context->load_verify_file(fullPath, code);
					if(code.value() != Asio::OK)
					{
						LOG_ERROR("load ssh : {}", fullPath);
						return nullptr;
					}
					socketProxy = this->mNetComponent->CreateSocket(*context);
					this->mSslContexts.emplace(fullPath, std::move(context));
				}
			}
			else
			{
				socketProxy = this->mNetComponent->CreateSocket(*iter->second);
			}
		}
		else
		{
			socketProxy = this->mNetComponent->CreateSocket();
		}
#else
		socketProxy = this->mNetComponent->CreateSocket();
#endif
		Asio::Context & main = this->mApp->GetContext();
		std::shared_ptr<http::Client> httpClient = std::make_shared<http::Client>(this, main);
		{
			httpClient->SetSocket(socketProxy);
		}
		return httpClient;
	}

	std::unique_ptr<http::Response> HttpComponent::Get(const std::string& url, int second)
	{
		std::unique_ptr<http::Request> request = std::make_unique<http::Request>("GET");
		if (!request->SetUrl(url))
		{
			LOG_ERROR("parse {} fail", url);
			return nullptr;
		}
		request->SetTimeout(second);
		std::unique_ptr<http::Response> response = this->Do(std::move(request));
		if (response != nullptr && response->Code() != HttpStatus::OK
			&& response->Code() != HttpStatus::FOUND)
		{
			LOG_ERROR("[GET] {} error={}", url, HttpStatusToString(response->Code()))
		}
		return response;
	}

	void HttpComponent::OnDelTask(int key)
	{
		this->mUseClients.Del(key);
	}

	int HttpComponent::Send(std::unique_ptr<http::Request> request, std::function<void(std::unique_ptr<http::Response>)> && cb)
	{
#ifndef __ENABLE_OPEN_SSL__
		if(request->IsHttps())
		{
			const http::Url & url = request->GetUrl();
			LOG_ERROR("[not ssl] => {}", url.ToStr());
			return XCode::SendMessageFail;
		}
#endif
		int rpcId = this->BuildRpcId();
		request->Header().SetKeepAlive(false);
		const http::Url & url = request->GetUrl();
		std::unique_ptr<http::Response> response = std::make_unique<http::Response>();
		std::shared_ptr<http::Client> httpAsyncClient = this->CreateClient(request.get());
		//LOG_DEBUG("connect {} server => {}:{}", url.Protocol(), url.Host(), url.Port());
		{
			this->mUseClients.Add(rpcId, httpAsyncClient);
			this->AddTask(new HttpCallbackTask(rpcId, cb));
			httpAsyncClient->Do(std::move(request), std::move(response), rpcId);
		}
		return XCode::Ok;
	}

	int HttpComponent::Send(std::unique_ptr<http::Request> request, std::unique_ptr<http::Response> response, int& rpcId)
	{
#ifndef __ENABLE_OPEN_SSL__
		if(request->IsHttps())
		{
			const http::Url & url = request->GetUrl();
			LOG_ERROR("[not ssl] => {}", url.ToStr());
			return XCode::SendMessageFail;
		}
#endif
		rpcId = this->BuildRpcId();
		request->Header().SetKeepAlive(false);
		std::shared_ptr<http::Client> httpAsyncClient = this->CreateClient(request.get());
		//LOG_DEBUG("connect {} server => {}:{}", url.Protocol(), url.Host(), url.Port());
		{
			this->mUseClients.Add(rpcId, httpAsyncClient);
			httpAsyncClient->Do(std::move(request), std::move(response), rpcId);
		}
		return XCode::Ok;
	}

	std::unique_ptr<http::Response> HttpComponent::Do(std::unique_ptr<http::Request> request)
	{
		std::unique_ptr<http::Response> response = std::make_unique<http::Response>();
		{
			int taskId = 0;
			if(this->Send(std::move(request), std::move(response), taskId) != XCode::Ok)
			{
				return std::make_unique<http::Response>(HttpStatus::INTERNAL_SERVER_ERROR);
			}
			return this->BuildRpcTask<HttpRequestTask>(taskId)->Await();
		}
	}

	std::unique_ptr<http::Response> HttpComponent::Do(std::unique_ptr<http::Request> request, std::unique_ptr<http::Content> body)
	{
		std::unique_ptr<http::Response> response = std::make_unique<http::Response>();
		{
			int taskId = 0;
			response->SetContent(std::move(body));
			if(this->Send(std::move(request), std::move(response), taskId) != XCode::Ok)
			{
				return std::make_unique<http::Response>(HttpStatus::INTERNAL_SERVER_ERROR);
			}
			return this->BuildRpcTask<HttpRequestTask>(taskId)->Await();
		}
	}

	std::unique_ptr<http::Response> HttpComponent::Post(const std::string& url, const json::w::Document& document, int second)
	{
		std::unique_ptr<http::Request> request = std::make_unique<http::Request>("POST");
		if (!request->SetUrl(url))
		{
			return nullptr;
		}
		request->SetTimeout(second);
		request->SetContent(document);
		std::unique_ptr<http::Response>response = this->Do(std::move(request));
		if (response != nullptr && response->Code() != HttpStatus::OK)
		{
			LOG_ERROR("[POST] {} fail:{}", url, HttpStatusToString(response->Code()))
		}
		return response;
	}

	std::unique_ptr<http::Response> HttpComponent::Post(const std::string& url, const std::string& data, int second)
	{
		std::unique_ptr<http::Request> request = std::make_unique<http::Request>("POST");
		if (!request->SetUrl(url))
		{
			return nullptr;
		}
		request->SetTimeout(second);
		request->SetContent(http::Header::JSON, data);
		std::unique_ptr<http::Response> response = this->Do(std::move(request));
		if (response != nullptr && response->Code() != HttpStatus::OK)
		{
			LOG_ERROR("[POST] {} fail:{}", url, HttpStatusToString(response->Code()))
		}
		return response;
	}

	void HttpComponent::OnMessage(http::Request* request, http::Response* response) noexcept
	{
		int taskId = 0;
		delete request;
		response->Header().Del("t", taskId);
		this->OnResponse(taskId, std::unique_ptr<http::Response>(response));
	}
}