//
// Created by yy on 2024/10/9.
//

#include "LuaOss.h"
#include "Entity/Actor/App.h"
#include "Yyjson/Lua/ljson.h"
#include "Oss/Task/LuaOssTask.h"
#include "Http/Common/HttpResponse.h"
#include "Oss/Component/OssComponent.h"
#include "Http/Component/HttpComponent.h"

int oss::Sign(lua_State* L)
{
	std::string json;
	acs::OssComponent* ossComponent = acs::App::Get<acs::OssComponent>();
	{
		json::r::Document document;
		lua::yyjson::read(L, 1, json);
		if (!document.Decode(json))
		{
			return 0;
		}
		oss::Policy ossPolicy;
		{
			document.Get("file_name", ossPolicy.file_name);
			document.Get("file_type", ossPolicy.file_type);
			document.Get("max_length", ossPolicy.max_length);
			document.Get("expiration", ossPolicy.expiration);
			document.Get("limit_type", ossPolicy.limit_type);
			document.Get("upload_dir", ossPolicy.upload_dir);
		}
		json::w::Document result;
		ossComponent->Sign(ossPolicy, result);

		result.Encode(&json);
		lua::yyjson::write(L, json.c_str(), json.size());
	}
	return 1;
}

int oss::Upload(lua_State* L)
{
	lua_pushthread(L);
	std::string path(luaL_checkstring(L, 1));
	std::string uploadDir(luaL_checkstring(L, 2));
	acs::OssComponent* ossComponent = acs::App::Get<acs::OssComponent>();
	acs::HttpComponent* httpComponent = acs::App::Get<acs::HttpComponent>();
	{
		std::unique_ptr<http::Request> request = ossComponent->New(path, uploadDir);
		std::unique_ptr<http::Response> response1 = std::make_unique<http::Response>();
		if(request == nullptr)
		{
			luaL_error(L, "create request fail");
			return 0;
		}
		if(!lua_isthread(L, -1))
		{
			return 0;
		}
		int rpcId = 0;
		const std::string url = request->GetUrl().ToStr();
		httpComponent->Send(std::move(request), std::move(response1), rpcId);
		return httpComponent->AddTask(new acs::LuaOssRequestTask(rpcId, L, url))->Await();
	}
}
