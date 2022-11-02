//
// Created by mac on 2022/5/30.
//

#include"LuaHttp.h"
#include"Json/Lua/Json.h"
#include"File/DirectoryHelper.h"
#include"Lua/UserDataParameter.h"
#include"Component/HttpComponent.h"
#include"Client/HttpRequestClient.h"
#include"Component/ProtoComponent.h"

#include"Http/HttpRequest.h"
#include"Task/HttpTask.h"
using namespace Sentry;
namespace Lua
{
	int HttpClient::Get(lua_State* lua)
    {
        lua_pushthread(lua);
        if (!lua_isuserdata(lua, 1))
        {
            luaL_error(lua, "first parameter must httpComponent point");
            return 0;
        }
        if (!lua_isstring(lua, 2))
        {
            luaL_error(lua, "url must string point");
        }
        size_t size = 0;
        const char *str = luaL_checklstring(lua, 2, &size);
        HttpComponent *httpComponent = UserDataParameter::Read<HttpComponent *>(lua, 1);
        std::shared_ptr<Http::GetRequest> getRequest(new Http::GetRequest());
        if (!getRequest->SetUrl(std::string(str, size)))
        {
            luaL_error(lua, "parse get url : [%s] failure", str);
            return 0;
        }
#ifdef __DEBUG__
        LOG_DEBUG("[http GET] url = " << std::string(str, size));
#endif
        std::shared_ptr<LuaHttpRequestTask> luaHttpTask(new LuaHttpRequestTask(lua));
        std::shared_ptr<HttpRequestClient> requestClient = httpComponent->CreateClient();

        long long id = requestClient->Do(getRequest);
        return httpComponent->AddTask(id, luaHttpTask)->Await(requestClient);
    }

	int HttpClient::Post(lua_State* lua)
    {
        size_t size = 0;
        lua_pushthread(lua);
        const char *str = luaL_checklstring(lua, 2, &size);
        HttpComponent *httpComponent = UserDataParameter::Read<HttpComponent *>(lua, 1);
        std::shared_ptr<Http::PostRequest> postRequest(new Http::PostRequest());
        if (!postRequest->SetUrl(std::string(str, size)))
        {
            luaL_error(lua, "parse post url : [%s] failure", str);
            return 0;
        }
        if (lua_isstring(lua, 3))
        {
            const char *data = luaL_checklstring(lua, 3, &size);
            postRequest->Json(data, size);
        }
        else if (lua_istable(lua, 3))
        {
            std::string json;
            Lua::Json::Read(lua, 3, &json);
            postRequest->Json(json.c_str(), json.size());
        }
        else
        {
            luaL_error(lua, "post parameter error");
            return 0;
        }

        std::shared_ptr<LuaHttpRequestTask> luaHttpTask(new LuaHttpRequestTask(lua));
        std::shared_ptr<HttpRequestClient> requestClient = httpComponent->CreateClient();

#ifdef __DEBUG__
        LOG_DEBUG("[http POST] url = " << std::string(str, size) << " data = " << postRequest->Content());
#endif
        long long id = requestClient->Do(postRequest);
        return httpComponent->AddTask(id, luaHttpTask)->Await(requestClient);
    }

	int HttpClient::Download(lua_State* lua)
	{
        return 0;
	}
}