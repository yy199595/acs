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
#include"Util/Tools/TimeHelper.h"
#include"Lua/Engine/ModuleClass.h"
#include "Rpc/Component/DispatchComponent.h"

#include "Rpc/Service/RpcService.h"
namespace acs
{
	HttpWebComponent::HttpWebComponent()
	{
		this->mConfig.Auth = true;
		this->mConfig.RpcDebug = false;
		this->mDispatch = nullptr;
		this->mCorComponent = nullptr;
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

		std::vector<RpcService*> rpcServices;
		this->mApp->GetComponents(rpcServices);
		for (RpcService* rpcService: rpcServices)
		{
			const std::string& name = rpcService->GetName();
			this->mRpcServices.Add(name, rpcService);
		}

		this->mCorComponent = App::Coroutine();
		this->mApp->GetComponents(this->mRecordComponents);
		this->mDispatch = this->GetComponent<DispatchComponent>();
		return this->ReadHttpConfig();
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
		if (jsonObject->Get("root", root))
		{
			this->AddRootDirector(root);
		}
		std::string token;
		jsonObject->Get("auth", this->mConfig.Auth);
		jsonObject->Get("index", this->mConfig.Index);
		jsonObject->Get("upload", this->mConfig.Upload);
		jsonObject->Get("domain", this->mConfig.Domain);
		this->mConfig.Index = fmt::format("{}/{}", root, this->mConfig.Index);
		return true;
	}


	void HttpWebComponent::OnReadHead(http::Request* request, http::Response* response)
	{
		int sockId = request->GetSockId();
		HttpStatus httpStatus = HttpStatus::OK;
		auto iter = this->mDefaultHeader.begin();
		for (; iter != this->mDefaultHeader.end(); iter++)
		{
			const std::string& key = iter->first;
			response->Header().Add(key, iter->second);
		}
		do
		{
			if (request->ConstHeader().Has("Upgrade"))
			{
				httpStatus = HttpStatus::UPGRADE_REQUIRED;
				break;
			}
			const std::string& path = request->GetUrl().Path();
			const HttpMethodConfig* httpConfig = HttpConfig::Inst()->GetMethodConfig(path);
			if (httpConfig == nullptr)
			{
				httpStatus = this->OnNotFound(request, response);
				if (httpStatus == HttpStatus::OK)
				{
					break;
				}
				if(this->mDispatch == nullptr)
				{
					httpStatus = HttpStatus::NOT_FOUND;
					break;
				}
				const RpcMethodConfig* rpcConfig = RpcConfig::Inst()->GetMethodByUrl(path);
				if (rpcConfig == nullptr)
				{
					httpStatus = HttpStatus::NOT_FOUND;
					break;
				}
				if (request->IsMethod("GET"))
				{
					this->OnApi(rpcConfig, request, response);
					return;
				}
				this->ReadMessageBody(sockId, std::make_unique<http::TextContent>());
				return;
			}

			if (!httpConfig->Open)
			{
				httpStatus = HttpStatus::NOT_FOUND;
				break;
			}
			if (!request->IsMethod(httpConfig->Type))
			{
				if (request->IsMethod("OPTIONS"))
				{
					response->Header().Add("Access-Control-Allow-Methods", httpConfig->Type);
					response->Header().Add("Access-Control-Allow-Headers", httpConfig->Headers);
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

			if (this->mConfig.Auth && httpConfig->Auth)
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
			const acs::HttpMethodConfig* httpConfig, http::Request* request)
	{
		std::string cont_type;
		if (!request->Header().GetContentType(cont_type))
		{
			return HttpStatus::BAD_REQUEST;
		}
		if (!httpConfig->Content.empty())
		{
			if (httpConfig->Content.find(cont_type) == std::string::npos)
			{
				const http::Url& url = request->GetUrl();
				LOG_ERROR("[{}]({}) {}:{}", url.Method(), url.Path(), cont_type, httpConfig->Content);
				return HttpStatus::BAD_REQUEST;
			}
		}
		std::unique_ptr<http::Content> body;
		if (!this->mFactory.New(cont_type, body))
		{
			return HttpStatus::UNSUPPORTED_MEDIA_TYPE;
		}
		long long contentLength = 0;
		if (!request->ConstHeader().GetContentLength(contentLength))
		{
			return HttpStatus::LENGTH_REQUIRED;
		}
		if (httpConfig->Limit > 0 && contentLength >= httpConfig->Limit)
		{
			return HttpStatus::BAD_REQUEST;
		}
		if (body->GetContentType() == http::ContentType::MULTIPAR)
		{
			if (this->mConfig.Upload.empty())
			{
				return HttpStatus::BAD_REQUEST;
			}
			const int limit = httpConfig->Limit;
			const std::string& path = this->mConfig.Upload;
			body->Cast<http::MultipartFromContent>()->Init(path, limit);
		}
		else if (body->GetContentType() == http::ContentType::FILE)
		{
			if (this->mConfig.Upload.empty())
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
			const std::string& upload = this->mConfig.Upload;
			const std::string path = fmt::format("{}/{}.{}", upload, guid, type);
			if (!body->Cast<http::FileContent>()->MakeFile(path))
			{
				return HttpStatus::INTERNAL_SERVER_ERROR;
			}
		}
		request->SetBody(std::move(body));
		return HttpStatus::OK;
	}

	HttpStatus HttpWebComponent::OnNotFound(http::Request* request, http::Response* response)
	{
		const std::string& path = request->GetUrl().Path();
		if (path == "/" && !this->mConfig.Index.empty())
		{
			response->File(http::Header::HTML, this->mConfig.Index);
			return HttpStatus::OK;
		}
		std::string filePath;
		for (const std::string& dir: this->mRoots)
		{
			filePath = fmt::format("{}/{}", dir, path);
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

	void HttpWebComponent::OnMessage(http::Request* request, http::Response* response)
	{
		const std::string& path = request->GetUrl().Path();
		//LOG_INFO("[{}] {}", path, request->GetBody()->ToStr())
		const HttpMethodConfig* httpConfig = HttpConfig::Inst()->GetMethodConfig(path);
		if (httpConfig != nullptr)
		{
			this->OnApi(httpConfig, request, response);
			return;
		}
		const RpcMethodConfig * rpcMethodConfig = RpcConfig::Inst()->GetMethodByUrl(path);
		if(rpcMethodConfig != nullptr)
		{
			this->OnApi(rpcMethodConfig, request, response);
			return;
		}
		std::string ip;
		request->Header().Get(http::Header::RealIp, ip);
		LOG_ERROR("[{}:{}] {}", ip, request->GetUrl().Method(), request->ToString());
		this->SendResponse(request->GetSockId(), HttpStatus::NOT_FOUND);
	}

	void HttpWebComponent::OnApi(const acs::HttpMethodConfig* httpConfig,
			http::Request* request, http::Response* response)
	{
		if (!httpConfig->IsAsync)
		{
			this->Invoke(httpConfig, request, response);
			return;
		}
		this->mCorComponent->Start(&HttpWebComponent::Invoke, this, httpConfig, request, response);
	}

	void HttpWebComponent::OnApi(const acs::RpcMethodConfig* config, http::Request* request, http::Response* response)
	{
		std::vector<std::string> keys;
		request->GetUrl().GetQuery().Get(keys);
		const http::FromContent& fromContent = request->GetUrl().GetQuery();
		rpc::Packet* message = new rpc::Packet();
		{
			std::string value;
			message->SetNet(rpc::Net::Http);
			message->SetType(rpc::Type::Request);
			for (const std::string& key: keys)
			{
				if (fromContent.Get(key, value))
				{
					message->GetHead().Add(key, value);
				}
			}
			message->SetSockId(request->GetSockId());
			message->GetHead().Add(rpc::Header::func, config->FullName);
		}
		const http::Content* content = request->GetBody();
		if (config->Proto > rpc::Porto::None && content != nullptr)
		{
			message->SetProto(rpc::Porto::Json);
			const http::TextContent* textContent = content->To<const http::TextContent>();
			if (textContent != nullptr)
			{
				message->Body()->assign(textContent->Content());
			}
		}
		int sockId = request->GetSockId();
		std::function<void(int code)> callback = [message, this, sockId](int code)
		{
			const std::string& desc = CodeConfig::Inst()->GetDesc(code);
			std::unique_ptr<http::JsonContent> jsonContent = std::make_unique<http::JsonContent>();
			{
				json::w::Document document;
				document.Add("code", code);
				document.Add("message", desc);
				if (message->GetBody().length() > 0)
				{
					document.AddJson("data", message->GetBody());
				}
				jsonContent->Write(document);
			}
			delete message;
			this->SendResponse(sockId, HttpStatus::OK, std::move(jsonContent));
		};
		int code = XCode::Ok;
		do
		{
			RpcService* logicService = this->mRpcServices.Find(config->Service);
			if (logicService == nullptr)
			{
				code = XCode::CallServiceNotFound;
				break;
			}
			if(config->IsAsync)
			{
				this->mCorComponent->Start([logicService, callback, config, message]()
				{
					callback(logicService->Invoke(config, message));
				});
				return;
			}
			code = logicService->Invoke(config, message);
		}
		while(false);
		callback(code);
	}

	HttpStatus HttpWebComponent::AuthToken(const HttpMethodConfig* config, http::Request* request)
	{
		std::string token;
		http::Head& head = request->Header();
		if (!head.Get(http::Header::Auth, token))
		{
			if (!head.Get("authorization", token))
			{
				LOG_WARN("not token field")
				return HttpStatus::UNAUTHORIZED;
			}
		}
		if (!config->Token.empty() && token == config->Token)
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
		if (httpToken.Permission < config->Permission)
		{
			LOG_ERROR("[{}] user_id:{} permission:{}/{}", config->Path,
					httpToken.UserId, httpToken.Permission, config->Permission);
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

	void HttpWebComponent::Invoke(const HttpMethodConfig* config, http::Request* request, http::Response* response)
	{
		HttpStatus code = HttpStatus::OK;
		for (HttpHandlerComponent* recordComponent: this->mRecordComponents)
		{
			int status = recordComponent->OnRequest(*config, *request);
			if (status != (int)HttpStatus::OK && status != XCode::Ok)
			{
				this->SendResponse(request->GetSockId(), (HttpStatus)status);
				return;
			}
		}
		do
		{
			http::Head& head = request->Header();
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

		for (HttpHandlerComponent* recordComponent: this->mRecordComponents)
		{
			recordComponent->OnRequestDone(*config, *request, *response);
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
			data->Add("client", (int)this->mHttpClients.Size());
			data->Add("wait", (int)(this->mWaitSockets.Size() + this->mHttpClients.Size()));
		}
	}
}