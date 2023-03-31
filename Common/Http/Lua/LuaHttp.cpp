//
// Created by mac on 2022/5/30.
//

#include"LuaHttp.h"
#include"Util/Json/Lua/Json.h"
#include"Util/File/DirectoryHelper.h"
#include"Http/Component/HttpComponent.h"
#include"Http/Client/HttpRequestClient.h"
#include"Proto/Component/ProtoComponent.h"

#include"Http/Common/HttpRequest.h"
#include"Http/Task/HttpTask.h"
#include"Entity/App/App.h"
using namespace Sentry;
namespace Lua
{
	int HttpClient::Get(lua_State* lua)
    {
        lua_pushthread(lua);        
        HttpComponent* httpComponent = App::Inst()->GetComponent<HttpComponent>();
        if (httpComponent == nullptr)
        {
            luaL_error(lua, "HttpComponent Is Null");
            return 0;
        }
        size_t size = 0;
        const char* str = luaL_checklstring(lua, 1, &size);
        std::shared_ptr<Http::GetRequest> getRequest = std::make_shared<Http::GetRequest>();
        if (!getRequest->SetUrl(std::string(str, size)))
        {
            luaL_error(lua, "parse get url : [%s] failure", str);
            return 0;
        }
		std::shared_ptr<LuaHttpRequestTask> luaHttpTask
				= std::make_shared<LuaHttpRequestTask>(lua);
#ifdef __DEBUG__
        LOG_DEBUG("[http GET] url = " << std::string(str, size));
#endif
		int taskId = 0;
		httpComponent->Send(getRequest, taskId);
        return httpComponent->AddTask(taskId, luaHttpTask)->Await();
    }

	int HttpClient::Post(lua_State* lua)
    {
        lua_pushthread(lua);
        HttpComponent *httpComponent = App::Inst()->GetComponent<HttpComponent>();
        if (httpComponent == nullptr)
        {
            luaL_error(lua, "HttpComponent Is Null");
            return 0;
        }
        size_t size = 0;
        const char* str = luaL_checklstring(lua, 1, &size);
        std::shared_ptr<Http::PostRequest> postRequest = std::make_shared<Http::PostRequest>();
        if (!postRequest->SetUrl(std::string(str, size)))
        {
            luaL_error(lua, "parse post url : [%s] failure", str);
            return 0;
        }
        if (lua_isstring(lua, 2))
        {
            const char *data = luaL_checklstring(lua, 2, &size);
            postRequest->Json(data, size);
        }
        else if (lua_istable(lua, 2))
        {
            std::string json;
            Lua::RapidJson::Read(lua, 2, &json);
            postRequest->Json(json.c_str(), json.size());
        }
        else
        {
            luaL_error(lua, "post parameter error");
            return 0;
        }
        std::shared_ptr<LuaHttpRequestTask> luaHttpTask(new LuaHttpRequestTask(lua));
#ifdef __DEBUG__
        //LOG_DEBUG("[http POST] url = " << std::string(str, size) << " data = " << postRequest->Content());
#endif
		int taskId = 0;
		httpComponent->Send(postRequest, taskId);
        return httpComponent->AddTask(taskId, luaHttpTask)->Await();
    }

	int HttpClient::Download(lua_State* lua)
	{
        return 0;
	}
}