//
// Created by mac on 2022/5/30.
//

#include"LuaHttp.h"
#include"File/DirectoryHelper.h"
#include"Lua/UserDataParameter.h"
#include"Lua/LuaWaitTaskSource.h"
#include"Client/HttpAsyncRequest.h"
#include"Component/HttpComponent.h"
#include"Client/HttpRequestClient.h"
#include"Component/ProtoComponent.h"
using namespace Sentry;
namespace Lua
{
	int Http::Get(lua_State* lua)
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
		const char * str = luaL_checklstring(lua, 2, &size);
		HttpComponent* httpComponent = UserDataParameter::Read<HttpComponent*>(lua, 1);
		std::shared_ptr<HttpGetRequest> getRequest = HttpGetRequest::Create(std::string(str, size));
		if(getRequest == nullptr)
		{
			luaL_error(lua, "parse get url : [%s] failure", str);
			return 0;
		}
#ifdef __DEBUG__
        LOG_DEBUG("[http get] url = " << std::string(str, size));
#endif
        std::shared_ptr<LuaHttpTask> luaHttpTask = getRequest->MakeLuaTask(lua, 0);
        std::shared_ptr<HttpRequestClient> requestClient = httpComponent->CreateClient();

        httpComponent->AddTask(luaHttpTask);
        requestClient->Request(getRequest);
        return luaHttpTask->Await(requestClient);
	}

	int Http::Post(lua_State* lua)
	{
		size_t size = 0;
		const char * str = luaL_checklstring(lua, 2, &size);
		std::shared_ptr<LuaWaitTaskSource> luaWaitTaskSource(new LuaWaitTaskSource(lua));
		HttpComponent* httpComponent = UserDataParameter::Read<HttpComponent*>(lua, 1);
		std::shared_ptr<HttpPostRequest> postRequest = HttpPostRequest::Create(std::string(str, size));
		if(postRequest == nullptr)
		{
			luaL_error(lua, "parse post url : [%s] failure", str);
			return 0;
		}
        const char * data = luaL_checklstring(lua, 3, &size);
		postRequest->AddBody(data, size);
        std::shared_ptr<HttpRequestClient> requestClient = httpComponent->CreateClient();
        std::shared_ptr<LuaHttpTask> luaHttpTask = postRequest->MakeLuaTask(lua, 0);

#ifdef __DEBUG__
        //LOG_DEBUG("[http post] url = " << std::string(str, size) << " data = " << postRequest->GetBody());
#endif

        httpComponent->AddTask(luaHttpTask);
        requestClient->Request(postRequest);
        return luaHttpTask->Await(requestClient);
	}

	int Http::Download(lua_State* lua)
	{
        HttpComponent* httpComponent = UserDataParameter::Read<HttpComponent*>(lua, 1);
        if(httpComponent == nullptr)
        {
            return 0;
        }

        lua_pushthread(lua);
        const char * url = luaL_checkstring(lua, 2);
        const char * path = luaL_checkstring(lua, 3);

		std::string dir, fileName;
		if(Helper::Directory::GetDirAndFileName(path, dir, fileName))
		{
			if(!Helper::Directory::MakeDir(dir))
			{
				luaL_error(lua, "make dir error : %s", dir.c_str());
				return 0;
			}
		}

		std::shared_ptr<HttpGetRequest> getRequest = HttpGetRequest::Create(url);
		if(getRequest == nullptr)
		{
			luaL_error(lua, "parse get url : [%s] failure", url);
			return 0;
		}

		std::fstream * fs = new std::fstream ();
		fs->open(path,  std::ios::out);
		if(!fs->is_open())
		{
			delete fs;
			luaL_error(lua, "open %s error", path);
			return 0;
		}
        std::shared_ptr<HttpRequestClient> requestClient = httpComponent->CreateClient();
        std::shared_ptr<LuaHttpTask> luaHttptask = getRequest->MakeLuaTask(lua, 0);
        requestClient->Request(getRequest, fs);

        return luaHttptask->Await(requestClient);
	}
}