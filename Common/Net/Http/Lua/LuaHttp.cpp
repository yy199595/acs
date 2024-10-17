//
// Created by mac on 2022/5/30.
//

#include"LuaHttp.h"
#include"Util/Crypt/md5.h"
#include"Yyjson/Lua/ljson.h"
#include"Util/File/DirectoryHelper.h"
#include"Http/Component/HttpComponent.h"
#include"Http/Client/RequestClient.h"
#include"Proto/Component/ProtoComponent.h"

#include"Http/Common/HttpRequest.h"
#include"Http/Task/HttpTask.h"
#include"Entity/Actor/App.h"
#include"Lua/Engine/UserDataParameter.h"
#include"Http/Component/HttpWebComponent.h"

using namespace acs;
namespace Lua
{
	int HttpHead::Get(lua_State* L)
	{
		http::Head * head = Lua::UserDataParameter::Read<http::Head*>(L, 1);
		const char * key = luaL_checkstring(L, 2);
		if(head == nullptr || key == nullptr)
		{
			return 0;
		}
		std::string value;
		if(!head->Get(key, value))
		{
			return 0;
		}
		lua_pushlstring(L, value.c_str(), value.size());
		return 1;
	}

	int HttpClient::Do(lua_State* lua)
	{
		static HttpComponent* httpComponent = nullptr;
		if (httpComponent == nullptr)
		{
			httpComponent = App::Get<HttpComponent>();
			if (httpComponent == nullptr)
			{
				luaL_error(lua, "HttpComponent Is Null");
				return 0;
			}
		}
		size_t size = 0;
		const char * method = luaL_checkstring(lua, 1);
		const char * url = luaL_checklstring(lua, 2, &size);
		std::unique_ptr<http::Request> request = std::make_unique<http::Request>(method);
		if(!request->SetUrl(std::string(url, size)))
		{
			luaL_error(lua, "parse get url : [%s] failure", url);
			return 0;
		}
		if(lua_istable(lua, 3))
		{
			lua_pushnil(lua);
			while (lua_next(lua, 3))
			{
				switch(lua_type(lua, -2))
				{
					case LUA_TSTRING:
					{
						const char* key = lua_tostring(lua, -2);
						const char * val = lua_tostring(lua, -1);
						request->Header().Add(key, val);
					}
						break;
					case LUA_TNUMBER:
					{
						const char* key = lua_tostring(lua, -2);
						long long val = lua_tointeger(lua, -1);
						request->Header().Add(key, (int)val);
					}
						break;
					default:
						luaL_error(lua, "unknown http head type");
						return 0;
				}
				lua_pop(lua, 1);
			}
		}

		if (lua_isstring(lua, 4))
		{
			const char *data = luaL_checklstring(lua, 4, &size);
			{
				std::string contentType = http::Header::TEXT;
				request->Header().Del(http::Header::ContentType, contentType);
				{
					request->SetContent(contentType.c_str(), data, size);
					request->Header().Add(http::Header::ContentLength, (int)size);
				}
			}
		}
		else if (lua_istable(lua, 4))
		{
			static std::string json;
			lua::yyjson::read(lua, 4, json);
			request->SetContent(http::Header::JSON, json);
		}
		int taskId = 0;
		lua_pushthread(lua);
		std::unique_ptr<http::Response> response = std::make_unique<http::Response>();

		httpComponent->Send(std::move(request), std::move(response), taskId);
		return httpComponent->AddTask(taskId, new LuaHttpRequestTask(lua))->Await();
	}

    int HttpClient::Get(lua_State* lua)
	{
		static HttpComponent* httpComponent = nullptr;
		if (httpComponent == nullptr)
		{
			httpComponent = App::Get<HttpComponent>();
			if (httpComponent == nullptr)
			{
				luaL_error(lua, "HttpComponent Is Null");
				return 0;
			}
		}

		size_t size = 0;
		lua_pushthread(lua);
		const char* str = luaL_checklstring(lua, 1, &size);
		std::unique_ptr<http::Response> response = std::make_unique<http::Response>();
		std::unique_ptr<http::Request> request = std::make_unique<http::Request>("GET");
		if (!request->SetUrl(std::string(str, size)))
		{
			luaL_error(lua, "parse get url : [%s] failure", str);
			return 0;
		}
		int taskId = 0;
		httpComponent->Send(std::move(request), std::move(response), taskId);
		return httpComponent->AddTask(taskId, new LuaHttpRequestTask(lua))->Await();
	}

	int HttpClient::Post(lua_State* lua)
    {
		static HttpComponent* httpComponent = nullptr;
		if (httpComponent == nullptr)
		{
			httpComponent = App::Get<HttpComponent>();
			if (httpComponent == nullptr)
			{
				luaL_error(lua, "HttpComponent Is Null");
				return 0;
			}
		}
        size_t size = 0;
		lua_pushthread(lua);
		const char* str = luaL_checklstring(lua, 1, &size);
		const std::string url(str, size);
        std::unique_ptr<http::Request> request = std::make_unique<http::Request>("POST");
        if (!request->SetUrl(url))
        {
            luaL_error(lua, "parse post url : [%s] failure", str);
            return 0;
        }
//#ifdef __DEBUG__
//		LOG_DEBUG("[http POST] url = {}", url);
//#endif

        if (lua_isstring(lua, 2))
        {
            const char *data = luaL_checklstring(lua, 2, &size);
            request->SetContent(http::Header::JSON, data, size);
        }
        else if (lua_istable(lua, 2))
        {
            std::string json;
			lua::yyjson::read(lua, 2, json);
			request->SetContent(http::Header::JSON, json);
		}
        else
        {
            //luaL_error(lua, "post parameter error");
			LOG_ERROR("[HTTP POST] {} data is nil", url);
            return 0;
        }
        std::unique_ptr<http::Response> response = std::make_unique<http::Response>();
#ifdef __DEBUG__
        //LOG_DEBUG("[http POST] url = " << std::string(str, size) << " data = " << postRequest->Content());
#endif
		int taskId = 0;
		httpComponent->Send(std::move(request), std::move(response), taskId);
        return httpComponent->AddTask(taskId, new LuaHttpRequestTask(lua))->Await();
    }

	int HttpClient::Upload(lua_State* lua)
	{
		HttpComponent* httpComponent = App::Get<HttpComponent>();
		if (httpComponent == nullptr)
		{
			luaL_error(lua, "HttpComponent Is Null");
			return 0;
		}
		size_t size, size2 = 0;
		const char* url = luaL_checklstring(lua, 1, &size);
		const char* path = luaL_checklstring(lua, 2, &size2);
		std::unique_ptr<http::Request> request = std::make_unique<http::Request>("POST");
		if (!request->SetUrl(std::string(url, size)))
		{
			luaL_error(lua, "parse post url : [%s] failure", url);
			return 0;
		}
		std::string type = http::Header::Bin;
		if(lua_isstring(lua, 3))
		{
			type = lua_tostring(lua, 3);
		}
		std::unique_ptr<http::FileContent> fileData = std::make_unique<http::FileContent>();
		{
			if(!fileData->OpenFile(path, type))
			{
				luaL_error(lua, "open file error : %s", path);
				return 0;
			}
		}
		request->SetBody(std::move(fileData));
		std::unique_ptr<http::Response> response = std::make_unique<http::Response>();

		int taskId = 0;
		lua_pushthread(lua);
		//request->Header().SetKeepAlive(false);
		httpComponent->Send(std::move(request), std::move(response), taskId);
		return httpComponent->AddTask(taskId, new LuaHttpRequestTask(lua))->Await();
	}

	int HttpClient::Download(lua_State* lua)
	{
        HttpComponent* httpComponent = App::Get<HttpComponent>();
        if (httpComponent == nullptr)
        {
            luaL_error(lua, "HttpComponent Is Null");
            return 0;
        }
        size_t size, size2 = 0;
        const char* url = luaL_checklstring(lua, 1, &size);
        const char* path = luaL_checklstring(lua, 2, &size2);
        std::unique_ptr<http::Request> request = std::make_unique<http::Request>("GET");
		std::unique_ptr<http::Response> response = std::make_unique<http::Response>();
		if(!response->OpenOrCreateFile(path))
		{
			luaL_error(lua, "open or create fail error : %s", path);
			return 0;
		}
		if (!request->SetUrl(std::string(url, size)))
        {
            luaL_error(lua, "parse post url : [%s] failure", url);
            return 0;
        }

        int taskId = 0;
		lua_pushthread(lua);
		request->Header().Add("Accept", "*/*");
		//request->Header().Add("User-Agent", "Chrome");
		//request->Header().Add("Accept-Encoding", "gzip, deflate, br");
        httpComponent->Send(std::move(request), std::move(response), taskId);
        return httpComponent->AddTask(taskId, new LuaHttpRequestTask(lua))->Await();
	}
}