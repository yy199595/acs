//
// Created by zmhy0073 on 2022/8/29.
//
#include"Admin.h"
#include"Util/Zip/Zip.h"
#include"Core/System/System.h"
#include"Entity/Actor/App.h"
#include"Util/File/DirectoryHelper.h"
#include"Message/com/com.pb.h"
#include"Util/File/FileHelper.h"
#include"Util/Time/TimeHelper.h"
#include"Auth/Jwt/Jwt.h"
#include"Proto/Component/ProtoComponent.h"
#include"Web/Component/AdminComponent.h"
#include"Mongo/Component/MongoComponent.h"
namespace joke
{
	Admin::Admin()
	{
		this->mProto = nullptr;
		this->mAdmin = nullptr;
		this->mActorComponent = nullptr;
	}

	bool Admin::Awake()
	{
		this->mApp->AddComponent<AdminComponent>();
		return true;
	}

    bool Admin::OnInit()
	{
		BIND_COMMON_HTTP_METHOD(Admin::Ping);
		BIND_COMMON_HTTP_METHOD(Admin::Info);
		BIND_COMMON_HTTP_METHOD(Admin::Stop);
		BIND_COMMON_HTTP_METHOD(Admin::Hello);
		BIND_COMMON_HTTP_METHOD(Admin::Hotfix);
		BIND_COMMON_HTTP_METHOD(Admin::Login);
		BIND_COMMON_HTTP_METHOD(Admin::Register);
		BIND_COMMON_HTTP_METHOD(Admin::RpcInterface);
		BIND_COMMON_HTTP_METHOD(Admin::HttpInterface);
		this->mProto = this->GetComponent<ProtoComponent>();
		this->mAdmin = this->GetComponent<AdminComponent>();
		this->mActorComponent = this->GetComponent<ActorComponent>();
		return true;
	}

	int Admin::Login(const http::Request& request, json::w::Document& response)
	{
		std::string user, password, ip;
		const http::FromData & query = request.GetUrl().GetQuery();
		{
			request.GetIp(ip);
			LOG_ERROR_CHECK_ARGS(query.Get("user", user))
			LOG_ERROR_CHECK_ARGS(query.Get("passwd", password))
		}
		std::unique_ptr<admin::UserInfo> userInfo = this->mAdmin->GetUserInfo(user);
		if(userInfo == nullptr)
		{
			return XCode::AccountNotExists;
		}
		if(userInfo->password != password)
		{
			return XCode::AccountPasswordError;
		}
		std::string payLoad;
		long long nowTime = help::Time::NowSec();
		long long tokenExpTime = nowTime + 86400;
		json::w::Document document;
		{
			document.Add("t", tokenExpTime);
			document.Add("u", userInfo->user_id);
			document.Add("p", userInfo->permission);
		}
		document.Encode(&payLoad);
		userInfo->login_ip = ip;
		userInfo->login_time = nowTime;
		std::string key = this->mApp->Config().GetSecretKey();
		const std::string token = jwt::Create(payLoad, key);
		std::unique_ptr<json::w::Value> jsonData = response.AddObject("data");
		{
			jsonData->Add("token", token);
			jsonData->Add("tick", tokenExpTime);
		}
		json::w::Document updater;
		updater.Add("last_ip", ip);
		updater.Add("login_time", nowTime);
		this->mAdmin->UpdateUser(userInfo->user_id, updater);
		std::unique_ptr<json::w::Value> jsonInfo = response.AddObject("info");
		{
			jsonInfo->Add("login_ip", ip);
			jsonInfo->Add("name", userInfo->name);
			jsonInfo->Add("city_name", userInfo->city_name);
			jsonInfo->Add("permission", userInfo->permission);
			jsonInfo->Add("login_time", userInfo->login_time);
			jsonInfo->Add("create_time", userInfo->create_time);
		}
		const_cast<http::FromData&>(query).Add(http::query::UserId, userInfo->user_id);
		return XCode::Ok;
	}

	int Admin::Register(const json::r::Document& request)
	{
		admin::UserInfo userInfo;
		LOG_ERROR_CHECK_ARGS(request.Get("city", userInfo.city))
		LOG_ERROR_CHECK_ARGS(request.Get("name", userInfo.name))
		LOG_ERROR_CHECK_ARGS(request.Get("account", userInfo.account))
		LOG_ERROR_CHECK_ARGS(request.Get("passwd", userInfo.password))
		return XCode::Ok;
	}

	int Admin::Ping(const http::FromData & request, json::w::Value & response)
	{
		int id = 0;
		if(!request.Get("id", id))
		{
			return XCode::CallArgsError;
		}
		Server* server = this->mActorComponent->GetServer(id);
		if(server == nullptr)
		{
			return XCode::NotFoundActor;
		}
		long long t1 = help::Time::NowMil();
		int code = server->Call("Node.Ping");
		{
			response.Add("time", help::Time::NowMil() - t1);
		}
		return code;
	}

    int Admin::Hello(http::Response& response)
	{
		response.SetContent(http::Header::TEXT, "hello");
		return XCode::Ok;
	}

	void Admin::AddRpcData(json::w::Value & response, const joke::RpcMethodConfig* methodConfig)
	{
		pb_json::JsonOptions options;
		options.add_whitespace = true;
		options.always_print_primitive_fields = true;
		response.Add("name", methodConfig->FullName.c_str());
		response.Add("service", methodConfig->Service.c_str());

		if(!methodConfig->Request.empty())
		{
			std::string json;
			pb::Message * request = this->mProto->Temp(methodConfig->Request);
			if(request && pb_json::MessageToJsonString(*request, &json, options).ok())
			{
				response.Add("req", json.c_str(), json.size());
			}
			response.Add("request", methodConfig->Request.c_str());
		}

		if(!methodConfig->Response.empty())
		{
			std::string json;
			pb::Message * request = this->mProto->Temp(methodConfig->Response);
			if(request && pb_json::MessageToJsonString(*request, &json, options).ok())
			{
				response.Add("res", json.c_str(), json.size());
			}
			response.Add("response", methodConfig->Response.c_str());
		}

		response.Add("client", methodConfig->IsClient);
		response.Add("open", methodConfig->IsOpen);
		response.Add("forward", methodConfig->Forward);
		response.Add("async", methodConfig->IsAsync);
	}

	int Admin::RpcInterface(const http::FromData & request, json::w::Document & response)
	{
		int page = 0;
		int type = 0;
		std::string service;
		request.Get("type", type);
		std::vector<const RpcMethodConfig *> methodConfigs;
		if(request.Get("name", service) && !service.empty())
		{
			RpcConfig::Inst()->GetMethodConfigs(service, methodConfigs);
		}
		else
		{
			if(!request.Get("page", page) || page < 0)
			{
				return XCode::CallArgsError;
			}
			page = page - 1;
			RpcConfig::Inst()->GetMethodConfigs(methodConfigs, type);
		}

		std::unique_ptr<json::w::Value> list = response.AddArray("list");
		for(int index = page * 10; index < (page * 10) + 10 && index < methodConfigs.size(); index++)
		{
			const RpcMethodConfig * methodConfig = methodConfigs[index];
			if(methodConfig != nullptr)
			{
				this->AddRpcData(*list->AddObject(), methodConfig);
			}
		}

		response.Add("count", (int)methodConfigs.size());
		return XCode::Ok;
	}

	int Admin::HttpInterface(const http::FromData & request, json::w::Document & response)
	{
		int page = 0;
		std::string method, name;
		std::vector<const HttpMethodConfig *> configs;
		if(request.Get("name", name) && !name.empty())
		{
			HttpConfig::Inst()->GetMethodList(configs, name);
		}
		else
		{
			request.Get("type", method);
			if(!request.Get("page", page) || page < 0)
			{
				return XCode::CallArgsError;
			}
			page--;
			HttpConfig::Inst()->GetMethodConfigs(configs, method);
		}

		std::unique_ptr<json::w::Value> list = response.AddArray("list");
		for(int index = page * 10; index < page * 10 + 10 && index < configs.size(); index++)
		{
			const HttpMethodConfig * methodConfig = configs.at(index);
			if(methodConfig != nullptr)
			{
				std::unique_ptr<json::w::Value> obj = list->AddObject();
				{
					obj->Add("url", methodConfig->Path);
					obj->Add("desc", methodConfig->Desc);
					obj->Add("method", methodConfig->Type);
					obj->Add("async", methodConfig->IsAsync);
					obj->Add("content", methodConfig->Content);
					obj->Add("request", methodConfig->Request);
					obj->Add("permiss", methodConfig->Permission);
					obj->Add("bind", fmt::format("{}.{}", methodConfig->Service, methodConfig->Method));
				}
			}
		}
		response.Add("count", (int)configs.size());
		return XCode::Ok;
	}

	int Admin::Hotfix(const http::FromData & request, json::w::Value&response)
	{
		int id = 0;
		if(!request.Get("id", id))
		{
			return XCode::CallArgsError;
		}
		Server* server = this->mActorComponent->GetServer(id);
		if(server == nullptr)
		{
			return XCode::NotFoundActor;
		}
		return server->Call("Node.Hotfix");
	}

	int Admin::Stop(const http::FromData & request, json::w::Value & response)
	{
		int id = 0;
		if(!request.Get("id", id))
		{
			response.Add("error", "not field:id");
			return XCode::CallArgsError;
		}
		Server* server = this->mActorComponent->GetServer(id);
		if(server == nullptr)
		{
			response.Add("error", "not find server");
			return XCode::NotFoundActor;
		}
		return server->Call("Node.Shutdown");
	}

	int Admin::Info(const http::FromData & request, json::w::Document& response)
    {
		int id = -1;
		std::vector<int> serverActors;
		if(request.Get("id", id) & id >= 0)
		{
			serverActors.emplace_back(id);
		}
		else
		{
			this->mActorComponent->GetServers(serverActors);
		}
		std::unique_ptr<json::w::Value> document = response.AddArray("list");
		std::unique_ptr<com::type::json> result = std::make_unique<com::type::json>();
		for(const int serverId : serverActors)
		{
			result->Clear();
			if(Server * server = this->mActorComponent->GetServer(serverId))
			{
				if (server->Call("Node.RunInfo", result.get()) == XCode::Ok)
				{
					document->PushJson(result->json());
				}
			}
		}
		return XCode::Ok;
    }
}