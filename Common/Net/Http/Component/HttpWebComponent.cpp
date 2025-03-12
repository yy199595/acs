//
// Created by zmhy0073 on 2022/6/21.
//

#include"HttpWebComponent.h"
#include"Entity/Actor/App.h"
#include"Core/System/System.h"
#include"Http/Client/HttpSession.h"
#include"Http/Service/HttpService.h"
#include"Util/File/DirectoryHelper.h"
#include"Util/File/FileHelper.h"
#include"Server/Config/CodeConfig.h"
#include"Util/Tools/TimeHelper.h"
#include"Lua/Engine/ModuleClass.h"
#include "Rpc/Component/DispatchComponent.h"

namespace acs
{
	HttpWebComponent::HttpWebComponent()
	{
		this->mRecord = nullptr;
		this->mConfig.index = "index.html";
		REGISTER_JSON_CLASS_FIELD(http::Config, auth);
		REGISTER_JSON_CLASS_FIELD(http::Config, root);
		REGISTER_JSON_CLASS_FIELD(http::Config, index);
		REGISTER_JSON_CLASS_FIELD(http::Config, upload);
		REGISTER_JSON_CLASS_FIELD(http::Config, domain);
		REGISTER_JSON_CLASS_FIELD(http::Config, whiteList);


		this->mCoroutine = nullptr;
		this->mFactory.Add<http::FromContent>(http::Header::FORM);
		this->mFactory.Add<http::JsonContent>(http::Header::JSON);

		this->mFactory.Add<http::TextContent>(http::Header::TEXT);
		this->mFactory.Add<http::TextContent>(http::Header::XML);
		this->mFactory.Add<http::TextContent>(http::Header::TEXT);

		this->mFactory.Add<http::FileContent>(http::Header::CSS);
		this->mFactory.Add<http::FileContent>(http::Header::HTML);
		this->mFactory.Add<http::FileContent>(http::Header::JPG);
		this->mFactory.Add<http::FileContent>(http::Header::PNG);
		this->mFactory.Add<http::FileContent>(http::Header::JPEG);

		this->mFactory.Add<http::BinContent>(http::Header::PB);

		this->mFactory.Add<http::MultipartFromContent>(http::Header::MulFromData);
	}

	bool HttpWebComponent::LateAwake()
	{
		std::vector<HttpService*> httpServices;
		this->mApp->GetComponents(httpServices);
		for (HttpService* httpService: httpServices)
		{
			const std::string& name = httpService->GetName();
			this->mHttpServices.Add(name, httpService);
		}

		this->mCoroutine = App::Coroutine();
		ServerConfig * config = ServerConfig::Inst();
		LOG_CHECK_RET_FALSE(config->Get("http", this->mConfig));
		if (!this->mConfig.root.empty())
		{
			this->AddRootDirector(this->mConfig.root);
			if(!this->mConfig.index.empty())
			{
				this->mPath = fmt::format("{}/{}", this->mConfig.root, this->mConfig.index);
			}
		}
		this->mRecord = this->mApp->GetComponent<IHttpRecordComponent>();
		return true;
	}

	bool HttpWebComponent::AddRootDirector(const std::string& dir)
	{
		LOG_CHECK_RET_FALSE(help::dir::DirectorIsExist(dir));
		auto iter = std::find(this->mRoots.begin(), this->mRoots.end(), dir);
		if (iter != this->mRoots.end())
		{
			return false;
		}
		this->mRoots.emplace_back(dir);
		return true;
	}

	void HttpWebComponent::OnReadHead(http::Request* request, http::Response* response) noexcept
	{
		int sockId = request->GetSockId();
		HttpStatus httpStatus = HttpStatus::OK;
		auto iter = this->mConfig.header.begin();
		for (; iter != this->mConfig.header.end(); iter++)
		{
			const std::string& key = iter->first;
			response->Header().Add(key, iter->second);
		}
		do
		{
			if (request->ConstHeader().Has("upgrade"))
			{
				httpStatus = HttpStatus::UPGRADE_REQUIRED;
				break;
			}
			const std::string& path = request->GetUrl().Path();
			const HttpMethodConfig* httpConfig = HttpConfig::Inst()->GetMethodConfig(path);
			if (httpConfig == nullptr)
			{
				if (this->OnNotFound(request, response) != HttpStatus::OK)
				{
					httpStatus = HttpStatus::NOT_FOUND;
					break;
				}
				return;
			}

			if (!httpConfig->open)
			{
				httpStatus = HttpStatus::NOT_FOUND;
				break;
			}
			if (!request->IsMethod(httpConfig->type))
			{
				if (request->IsMethod("OPTIONS"))
				{
					response->Header().Add("Access-Control-Allow-Methods", httpConfig->type);
					response->Header().Add("Access-Control-Allow-Headers", httpConfig->headers);
					break;
				}
				httpStatus = HttpStatus::METHOD_NOT_ALLOWED;
				break;
			}
			if (!request->IsMethod("GET"))
			{
				httpStatus = this->CreateHttpData(httpConfig, request);
				if(httpStatus != HttpStatus::OK)
				{
					break;
				}
			}

			if (this->mConfig.auth && httpConfig->auth)
			{
				if (this->AuthToken(httpConfig, request) != HttpStatus::OK)
				{
					httpStatus = HttpStatus::UNAUTHORIZED;
					break;
				}
			}
			if (!httpConfig->WhiteList.empty()) //白名单判断
			{
				std::string ip;
				request->GetIp(ip);
				if (httpConfig->WhiteList.find(ip) == httpConfig->WhiteList.end())
				{
					httpStatus = HttpStatus::NOT_FOUND;
					break;
				}
			}
			if (request->IsMethod("GET"))
			{
				this->OnApi(httpConfig, request, response);
				return;
			}
			this->ReadMessageBody(sockId);
			return;
		} while (false);
		this->SendResponse(sockId, httpStatus);
	}

	HttpStatus HttpWebComponent::CreateHttpData(
			const acs::HttpMethodConfig* httpConfig, http::Request* request) noexcept
	{
		std::string cont_type;
		if (!request->Header().GetContentType(cont_type))
		{
			return HttpStatus::BAD_REQUEST;
		}
		if (!httpConfig->content.empty())
		{
			if (httpConfig->content.find(cont_type) == std::string::npos)
			{
				const http::Url& url = request->GetUrl();
				LOG_ERROR("[{}]({}) {}:{}", url.Method(), url.Path(), cont_type, httpConfig->content);
				return HttpStatus::BAD_REQUEST;
			}
		}
		std::unique_ptr<http::Content> body;
		if (!this->mFactory.New(cont_type, body))
		{
			if(!httpConfig->content.empty())
			{
				return HttpStatus::UNSUPPORTED_MEDIA_TYPE;
			}
			body = std::make_unique<http::TextContent>();
		}
		long long contentLength = 0;
		if (!request->ConstHeader().GetContentLength(contentLength))
		{
			return HttpStatus::LENGTH_REQUIRED;
		}
		if (httpConfig->limit > 0 && contentLength >= httpConfig->limit)
		{
			return HttpStatus::BAD_REQUEST;
		}
		if (body->GetContentType() == http::ContentType::MULTIPAR)
		{
			if (this->mConfig.upload.empty())
			{
				return HttpStatus::BAD_REQUEST;
			}
			const int limit = httpConfig->limit;
			const std::string& path = this->mConfig.upload;
			body->Cast<http::MultipartFromContent>()->Init(path, limit);
		}
		else if (body->GetContentType() == http::ContentType::FILE)
		{
			if (this->mConfig.upload.empty())
			{
				return HttpStatus::BAD_REQUEST;
			}
			size_t pos = cont_type.find('/');
			if (pos == std::string::npos)
			{
				return HttpStatus::INTERNAL_SERVER_ERROR;
			}
			long long guid = this->mApp->MakeGuid();
			std::string type = cont_type.substr(pos + 1);
			const std::string& upload = this->mConfig.upload;
			const std::string path = fmt::format("{}/{}.{}", upload, guid, type);
			if (!body->Cast<http::FileContent>()->MakeFile(path))
			{
				return HttpStatus::INTERNAL_SERVER_ERROR;
			}
		}
		request->SetBody(std::move(body));
		return HttpStatus::OK;
	}

	HttpStatus HttpWebComponent::OnNotFound(http::Request* request, http::Response* response) noexcept
	{
		const std::string& path = request->GetUrl().Path();
		if (path == "/" && !this->mPath.empty())
		{
			response->File(http::Header::HTML, this->mPath);
			return HttpStatus::OK;
		}
		for (const std::string& dir: this->mRoots)
		{
			std::string filePath = fmt::format("{}/{}", dir, path);
			if (!help::fs::FileIsExist(filePath))
			{
				continue;
			}
			std::string contentType = http::Header::TEXT;
			size_t pos = filePath.find_last_of('.');
			if (pos != std::string::npos)
			{
				std::string t = filePath.substr(pos + 1);
				contentType = http::GetContentType(t);
			}
			response->File(contentType, filePath);
			return HttpStatus::OK;
		}
		return HttpStatus::NOT_FOUND;
	}

	void HttpWebComponent::OnMessage(http::Request* request, http::Response* response) noexcept
	{
		const std::string& path = request->GetUrl().Path();
		const HttpMethodConfig* httpConfig = HttpConfig::Inst()->GetMethodConfig(path);
		if (httpConfig != nullptr)
		{
			this->OnApi(httpConfig, request, response);
			return;
		}
		std::string ip;
		request->Header().Get(http::Header::RealIp, ip);
		LOG_ERROR("[{}:{}] {}", ip, request->GetUrl().Method(), request->ToString());
		this->SendResponse(request->GetSockId(), HttpStatus::NOT_FOUND);
	}

	void HttpWebComponent::OnApi(const acs::HttpMethodConfig* httpConfig,
			http::Request* request, http::Response* response) noexcept
	{
		if (!httpConfig->async)
		{
			this->Invoke(httpConfig, request, response);
			return;
		}
		this->mCoroutine->Start(&HttpWebComponent::Invoke, this, httpConfig, request, response);
	}

	HttpStatus HttpWebComponent::AuthToken(const HttpMethodConfig* config, http::Request* request) noexcept
	{
		std::string token;
		http::Head& head = request->Header();
		const static std::string AUTH("authorization");
		if (!head.Get(AUTH, token))
		{
			return HttpStatus::UNAUTHORIZED;
		}
		if (!config->token.empty() && token == config->token)
		{
			return HttpStatus::OK;
		}
#ifdef __ENABLE_OPEN_SSL__
		json::r::Document document;
		if (!this->mApp->DecodeSign(token, document))
		{
			return HttpStatus::UNAUTHORIZED;
		}

		http::Token httpToken;
		document.Get("u", httpToken.UserId);
		document.Get("c", httpToken.ClubId);
		document.Get("t", httpToken.ExpTime);
		document.Get("p", httpToken.Permission);
		if (httpToken.Permission < config->permission)
		{
			LOG_ERROR("[{}] user_id:{} permission:{}/{}", config->path,
					httpToken.UserId, httpToken.Permission, config->permission);
			return HttpStatus::NOT_FOUND;
		}
		long long nowTime = help::Time::NowSec();
		if (httpToken.ExpTime > 0 && nowTime >= httpToken.ExpTime)
		{
			LOG_WARN("[{}:{}] token exp time", token, httpToken.UserId)
			return HttpStatus::UNAUTHORIZED;
		}
		http::FromContent& query = const_cast<http::FromContent&>(request->GetUrl().GetQuery());
		{
			query.Set(http::query::UserId, httpToken.UserId);
			query.Set(http::query::ClubId, httpToken.ClubId);
			query.Set(http::query::Permission, httpToken.Permission);
		}
#endif
		return HttpStatus::OK;
	}

	void HttpWebComponent::Invoke(const HttpMethodConfig* config, http::Request* request, http::Response* response) noexcept
	{
		HttpStatus code = HttpStatus::OK;
		do
		{
			http::Head& head = request->Header();
			if (this->mConfig.auth && config->auth)
			{
				code = this->AuthToken(config, request);
				if (code != HttpStatus::OK)
				{
					break;
				}
			}
			HttpService* httpService = this->mHttpServices.Find(config->service);
			if (httpService == nullptr)
			{
				std::string ip;
				code = HttpStatus::NOT_FOUND;
				head.Get(http::Header::RealIp, ip);
				LOG_ERROR("[{}] {} {}", ip, request->GetUrl().Path(), HttpStatusToString(HttpStatus::NOT_FOUND));
				break;
			}
			int logicCode = httpService->Invoke(config, *request, *response);
			if (response->GetBody() == nullptr)
			{
				json::w::Document document;
				const std::string& desc = CodeConfig::Inst()->GetDesc(logicCode);
				{
					document.Add("error", desc);
					document.Add("code", logicCode);
					response->Json(document);
				}
			}
#ifdef __DEBUG__
			if (logicCode != XCode::Ok)
			{
				std::string ip;
				head.Get(http::Header::RealIp, ip);
				const http::Url& url = request->GetUrl();
				const std::string& desc = CodeConfig::Inst()->GetDesc(logicCode);
				LOG_WARN("({})[{}] code:{} => {}", url.Method(), url.ToStr(), logicCode, desc);
			}
#endif
		} while (false);
		if(config->record && this->mRecord != nullptr)
		{
			this->mRecord->OnRequestDone(*config, *request, *response);
		}
		this->SendResponse(request->GetSockId(), code);
	}

	void HttpWebComponent::OnRecord(json::w::Document& document)
	{
		std::unique_ptr<json::w::Value> data = document.AddObject("web");
		{
			data->Add("success", this->mSuccessCount);
			data->Add("failure", this->mFailureCount);
			data->Add("sum", this->mNumPool.CurrentNumber());
			data->Add("client", (int)this->mHttpClients.size());
			data->Add("wait", this->mHttpClients.size());
		}
	}
}