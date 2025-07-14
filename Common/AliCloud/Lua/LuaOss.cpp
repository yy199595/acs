//
// Created by yy on 2024/10/9.
//

#include "LuaOss.h"
#include "Entity/Actor/App.h"
#include "Yyjson/Lua/ljson.h"
#include "AliCloud/Task/LuaOssTask.h"
#include "Http/Common/HttpResponse.h"
#include "Http/Component/HttpComponent.h"
#include "AliCloud/Component/AliOssComponent.h"

int oss::Sign(lua_State* L)
{
	acs::AliOssComponent* ossComponent = acs::App::Get<acs::AliOssComponent>();
	{
		size_t count = 0;
		json::r::Document document;
		std::unique_ptr<char> json;
		if(!lua::yyjson::read(L, 1, json, count))
		{
			return false;
		}
		if (!document.Decode(json.get(), count, YYJSON_READ_INSITU))
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
		if(!result.Serialize(json, count))
		{
			return 0;
		}
		lua::yyjson::write(L, json.get(), count);
	}
	return 1;
}

int oss::Upload(lua_State* L)
{
	lua_pushthread(L);
	std::string path(luaL_checkstring(L, 1));
	std::string uploadDir(luaL_checkstring(L, 2));
	acs::HttpComponent* httpComponent = acs::App::Get<acs::HttpComponent>();
	acs::AliOssComponent* ossComponent = acs::App::Get<acs::AliOssComponent>();
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
		httpComponent->Send(request, response1, rpcId);
		return httpComponent->AddTask(new acs::LuaOssRequestTask(rpcId, L, url))->Await();
	}
}
