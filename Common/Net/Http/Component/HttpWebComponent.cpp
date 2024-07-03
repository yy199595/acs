//
// Created by zmhy0073 on 2022/6/21.
//

#include"HttpWebComponent.h"
#include"Entity/Actor/App.h"
#include"Core/System/System.h"
#include"Rpc/Config/ServiceConfig.h"
#include"Http/Client/SessionClient.h"
#include"Http/Service/HttpService.h"
#include"Util/File/DirectoryHelper.h"
#include"Util/File/FileHelper.h"
#include"Server/Config/CodeConfig.h"

#include"Lua/Engine/ModuleClass.h"
#include"Redis/Component/RedisComponent.h"

namespace joke
{
    HttpWebComponent::HttpWebComponent()
	{
		this->mConfig.Auth = true;
		this->mCorComponent = nullptr;
		this->mFactory.Add<http::FromData>(http::Header::FORM);
		this->mFactory.Add<http::JsonData>(http::Header::JSON);

		this->mFactory.Add<http::TextData>(http::Header::TEXT);
		this->mFactory.Add<http::TextData>(http::Header::XML);
		this->mFactory.Add<http::TextData>(http::Header::TEXT);

		this->mFactory.Add<http::FileData>(http::Header::CSS);
		this->mFactory.Add<http::FileData>(http::Header::HTML);
		this->mFactory.Add<http::FileData>(http::Header::JPG);
		this->mFactory.Add<http::FileData>(http::Header::PNG);
		this->mFactory.Add<http::FileData>(http::Header::JPEG);

		this->mFactory.Add<http::MultipartFromData>(http::Header::MulFromData);
	}

    bool HttpWebComponent::LateAwake()
    {
		std::vector<HttpService *> httpServices;
		this->mApp->GetComponents(httpServices);
		for(HttpService * httpService : httpServices)
		{
			const std::string & name = httpService->GetName();
			this->mHttpServices.Add(name, httpService);
		}
		this->mCorComponent = this->mApp->Coroutine();
		return this->ReadHttpConfig();
    }

	bool HttpWebComponent::AddRootDirector(const std::string& dir)
	{
		auto iter = std::find(this->mRoots.begin(), this->mRoots.end(), dir);
		if(iter != this->mRoots.end())
		{
			return false;
		}
		this->mRoots.emplace_back(dir);
		return true;
	}

	bool HttpWebComponent::ReadHttpConfig()
	{
		std::unique_ptr<json::r::Value> jsonObject;
		if (!this->mApp->Config().Get("http", jsonObject))
		{
			return false;
		}
		this->mDefaultHeader.clear();
		std::unique_ptr<json::r::Value> jsonHeader;
		if (jsonObject->Get("header", jsonHeader))
		{
			std::vector<const char*> keys;
			if (jsonHeader->GetKeys(keys) > 0)
			{
				std::unique_ptr<json::r::Value> value;
				for (const char* key: keys)
				{
					if (jsonHeader->Get(key, value))
					{
						this->mDefaultHeader.emplace(key, value->ToString());
					}
				}
			}
		}
		std::string root;
		if(jsonObject->Get("root", root))
		{
			this->AddRootDirector(root);
		}
		std::string token;
		jsonObject->Get("auth", this->mConfig.Auth);
		jsonObject->Get("index", this->mConfig.Index);
		jsonObject->Get("upload", this->mConfig.Upload);
		jsonObject->Get("domain", this->mConfig.Domain);
		this->mKey = this->mApp->Config().GetSecretKey();
		this->mConfig.Index = fmt::format("{}/{}", root, this->mConfig.Index);
		return true;
	}

	bool HttpWebComponent::OnListenOk(const char* name)
	{
		return true;
	}

	void HttpWebComponent::OnReadHead(http::Request* request, http::Response* response)
	{
		int sockId = request->GetSockId();
		auto iter = this->mDefaultHeader.begin();
		for (; iter != this->mDefaultHeader.end(); iter++)
		{
			const std::string& key = iter->first;
			response->Header().Add(key, iter->second);
		}
		const std::string& path = request->GetUrl().Path();
		//response->Header().Add("Date", help::Time::GetDateGMT());
		const HttpMethodConfig* httpConfig = HttpConfig::Inst()->GetMethodConfig(path);
		LOG_DEBUG("[{}:{}]", request->GetUrl().Method(), request->GetUrl().ToStr());
		if (httpConfig == nullptr)
		{
			this->OnNotFound(request, response);
			this->Send(sockId);
			return;
		}
		else
		{
			if (!request->IsMethod(httpConfig->Type))
			{
				if (request->IsMethod("OPTIONS"))
				{
					response->Header().Add("Access-Control-Allow-Methods", httpConfig->Type);
					response->Header().Add("Access-Control-Allow-Headers", httpConfig->Headers);
					this->Send(sockId, HttpStatus::OK);
					return;
				}
				this->Send(sockId, HttpStatus::METHOD_NOT_ALLOWED);
				return;
			}
			if(!request->IsMethod("GET"))
			{
				HttpStatus status = this->CreateHttpData(httpConfig, request);
				if (status != HttpStatus::OK)
				{
					this->Send(sockId, status);
					return;
				}
			}

			if (this->mConfig.Auth && httpConfig->Auth)
			{
				if(this->AuthToken(httpConfig, request) != HttpStatus::OK)
				{
					this->Send(sockId, HttpStatus::UNAUTHORIZED);
					return;
				}
			}
			if (request->IsMethod("GET"))
			{
				this->OnApi(httpConfig, request, response);
				return;
			}
			this->ReadMessageBody(request->GetSockId());
		}
	}

	HttpStatus HttpWebComponent::CreateHttpData(
			const joke::HttpMethodConfig* httpConfig, http::Request* request)
	{
		std::string cont_type;
		//LOG_WARN("{} => {}", httpConfig->Path, request->Header().ToString())
		if(!request->Header().GetContentType(cont_type))
		{
			return HttpStatus::BAD_REQUEST;
		}
		if(!httpConfig->Content.empty())
		{
			if (httpConfig->Content.find(cont_type) == std::string::npos)
			{
				const http::Url & url = request->GetUrl();
				LOG_ERROR("[{}]({}) {}:{}", url.Method(), url.Path(), cont_type, httpConfig->Content);
				return HttpStatus::BAD_REQUEST;
			}
		}
		std::unique_ptr<http::Data> body;
		if(!this->mFactory.New(cont_type, body))
		{
			return HttpStatus::UNSUPPORTED_MEDIA_TYPE;
		}
		long long contentLength = 0;
		if(!request->ConstHeader().GetContentLength(contentLength))
		{
			return HttpStatus::LENGTH_REQUIRED;
		}
		if(httpConfig->Limit > 0 && contentLength >= httpConfig->Limit)
		{
			return HttpStatus::BAD_REQUEST;
		}
		if(body->GetContentType() == http::ContentType::MULTIPAR)
		{
			if (this->mConfig.Upload.empty())
			{
				return HttpStatus::BAD_REQUEST;
			}
			const int limit = httpConfig->Limit;
			const std::string uuid = this->mApp->NewUuid();
			const std::string & path = this->mConfig.Upload;
			body->Cast<http::MultipartFromData>()->Init(path, uuid, limit);
		}
		else if(body->GetContentType() == http::ContentType::FILE)
		{
			if (this->mConfig.Upload.empty())
			{
				return HttpStatus::BAD_REQUEST;
			}
			size_t pos = cont_type.find('/');
			if(pos == std::string::npos)
			{
				return HttpStatus::INTERNAL_SERVER_ERROR;
			}
			long long guid = this->mApp->MakeGuid();
			std::string type = cont_type.substr(pos + 1);
			const std::string & upload = this->mConfig.Upload;
			const std::string path = fmt::format("{}/{}.{}", upload, guid, type);
			if(!body->Cast<http::FileData>()->MakeFile(path))
			{
				return HttpStatus::INTERNAL_SERVER_ERROR;
			}
		}
		request->SetBody(std::move(body));
		return HttpStatus::OK;
	}

	void HttpWebComponent::OnNotFound(http::Request* request, http::Response* response)
	{
		const std::string & path = request->GetUrl().Path();
		if(path == "/")
		{
			response->File(http::Header::HTML, this->mConfig.Index);
			return;
		}
		std::string filePath;
		for(const std::string & dir : this->mRoots)
		{
			filePath = fmt::format("{}/{}", dir, path);
			if(!help::fs::FileIsExist(filePath))
			{
				continue;
			}
			std::string contentType = http::Header::TEXT;
			size_t pos = filePath.find_last_of('.');
			if(pos != std::string::npos)
			{
				std::string t = filePath.substr(pos + 1);
				contentType = http::GetContentType(t);
			}
			response->File(contentType, filePath);
			return;
		}
		response->SetCode(HttpStatus::NOT_FOUND);
	}

    void HttpWebComponent::OnMessage(http::Request * request, http::Response * response)
	{
		const std::string& path = request->GetUrl().Path();
		//LOG_INFO("[{}] {}", path, request->GetBody()->ToStr())
		const HttpMethodConfig* httpConfig = HttpConfig::Inst()->GetMethodConfig(path);
		if (httpConfig != nullptr)
		{
			this->OnApi(httpConfig, request, response);
			return;
		}
		std::string ip;
		request->Header().Get(http::Header::RealIp, ip);
		LOG_ERROR("[{}:{}] {}", ip, request->GetUrl().Method(), request->ToString());
		this->Send(request->GetSockId(), HttpStatus::NOT_FOUND);
	}

	void HttpWebComponent::OnApi(const joke::HttpMethodConfig* httpConfig,
			http::Request* request, http::Response* response)
	{
		if (!httpConfig->IsAsync)
		{
			this->Invoke(httpConfig, request, response);
			return;
		}
		this->mCorComponent->Start(&HttpWebComponent::Invoke, this, httpConfig, request, response);
	}

	HttpStatus HttpWebComponent::AuthToken(const HttpMethodConfig * config, http::Request* request)
	{
		std::string token;
		http::Head& head = request->Header();
		if(!head.Get(http::Header::Auth, token))
		{
			if(!head.Get("authorization", token))
			{
				LOG_WARN("not token field")
				return HttpStatus::UNAUTHORIZED;
			}
		}
#ifdef __ENABLE_OPEN_SSL__
		json::r::Document document;
		if(!this->mApp->DecodeSign(token, document))
		{
			return HttpStatus::UNAUTHORIZED;
		}

		http::Token httpToken;
		document.Get("u", httpToken.UserId);
		document.Get("c", httpToken.ClubId);
		document.Get("t", httpToken.ExpTime);
		document.Get("p", httpToken.Permission);
		if(httpToken.Permission < config->Permission)
		{
			LOG_ERROR("[{}] user_id:{} permission:{}/{}", config->Path,
					httpToken.UserId, httpToken.Permission, config->Permission);
			return HttpStatus::NOT_FOUND;
		}
		long long nowTime = help::Time::NowSec();
		if(httpToken.ExpTime > 0 && nowTime >= httpToken.ExpTime)
		{
			LOG_WARN("[{}:{}] token exp time", token, httpToken.UserId)
			return HttpStatus::UNAUTHORIZED;
		}
		http::FromData & query = const_cast<http::FromData&>(request->GetUrl().GetQuery());
		{
			query.Set(http::query::UserId, httpToken.UserId);
			query.Set(http::query::ClubId, httpToken.ClubId);
			query.Set(http::query::Permission, httpToken.Permission);
		}
#endif
		return HttpStatus::OK;
	}

    void HttpWebComponent::Invoke(const HttpMethodConfig* config, http::Request * request, http::Response * response)
	{
		HttpStatus code = HttpStatus::OK;
		do
		{
			http::Head & head = request->Header();
			if (this->mConfig.Auth && config->Auth)
			{
				code = this->AuthToken(config, request);
				if (code != HttpStatus::OK)
				{
					break;
				}
			}
			HttpService* httpService = this->mHttpServices.Find(config->Service);
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
				const std::string & desc = CodeConfig::Inst()->GetDesc(logicCode);
				{
					document.Add("error", desc);
					document.Add("code", logicCode);
					response->Json(document);
				}
			}
#ifdef __DEBUG__
			if(logicCode != XCode::Ok)
			{
				std::string ip;
				head.Get(http::Header::RealIp, ip);
				const http::Url& url = request->GetUrl();
				const std::string& desc = CodeConfig::Inst()->GetDesc(logicCode);
				LOG_WARN("({})[{}] code:{} => {}", url.Method(), url.ToStr(), logicCode, desc);
			}
#endif
		} while (false);
		//LOG_INFO("{}", response->ToString())
		this->Send(request->GetSockId(), code);
	}

	void HttpWebComponent::OnRecord(json::w::Document& document)
	{
		std::unique_ptr<json::w::Value> data = document.AddObject("web");
		{
			data->Add("sum", this->mNumPool.CurrentNumber());
			data->Add("pool", (int)this->mClientPools.Size());
			data->Add("client", (int)this->mHttpClients.Size());
			data->Add("wait", (int)(this->mWaitSockets.Size() + this->mHttpClients.Size()));
		}
	}
}