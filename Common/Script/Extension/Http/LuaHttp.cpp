//
// Created by mac on 2022/5/30.
//

#include"LuaHttp.h"
#include"App/App.h"
#include"Util/DirectoryHelper.h"
#include"Script/UserDataParameter.h"
#include"Async/Lua/LuaWaitTaskSource.h"
#include"Network/Http/HttpAsyncRequest.h"
#include"Component/Http/HttpComponent.h"
#include"Network/Http/HttpRequestClient.h"
#include"Component/Scene/MessageComponent.h"
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
		const char * str = lua_tolstring(lua, 2, &size);
		HttpComponent* httpComponent = UserDataParameter::Read<HttpComponent*>(lua, 1);
		std::shared_ptr<HttpGetRequest> getRequest = HttpGetRequest::Create(std::string(str, size));
		if(getRequest == nullptr)
		{
			luaL_error(lua, "parse get url : [%s] failure", str);
			return 0;
		}

		TaskComponent* taskComponent = App::Get()->GetTaskComponent();
		std::shared_ptr<LuaWaitTaskSource> luaRpcTaskSource(new LuaWaitTaskSource(lua));
		taskComponent->Start([luaRpcTaskSource, httpComponent, getRequest]()
		{
			std::string json;
			std::shared_ptr<HttpRequestClient> requestClient = httpComponent->CreateClient();
			std::shared_ptr<HttpAsyncResponse> httpResponse = requestClient->Request(getRequest);
			if (httpResponse != nullptr)
			{
				luaRpcTaskSource->SetMessage(httpResponse->GetData());
				return;
			}
			luaRpcTaskSource->SetResult();
		});
		return luaRpcTaskSource->Await();
	}

	int Http::Post(lua_State* lua)
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
			return 0;
		}
		if (!lua_isstring(lua, 3))
		{
			luaL_error(lua, "content must string");
			return 0;
		}

		size_t size = 0;
		const char * str = lua_tolstring(lua, 2, &size);
		TaskComponent* taskComponent = App::Get()->GetTaskComponent();
		std::shared_ptr<LuaWaitTaskSource> luaWaitTaskSource(new LuaWaitTaskSource(lua));
		HttpComponent* httpComponent = UserDataParameter::Read<HttpComponent*>(lua, 1);
		std::shared_ptr<HttpPostRequest> postRequest = HttpPostRequest::Create(std::string(str, size));
		if(postRequest == nullptr)
		{
			luaL_error(lua, "parse post url : [%s] failure", str);
			return 0;
		}
		postRequest->AddBody(lua_tolstring(lua, 3, &size), size);
		taskComponent->Start([luaWaitTaskSource, httpComponent, postRequest]()
		{
			std::string json;
			std::shared_ptr<HttpRequestClient> requestClient = httpComponent->CreateClient();
			std::shared_ptr<HttpAsyncResponse> httpResponse = requestClient->Request(postRequest);
			if (httpResponse != nullptr)
			{
				luaWaitTaskSource->SetMessage(httpResponse->GetData());
				return;
			}
			luaWaitTaskSource->SetResult();
		});
		return luaWaitTaskSource->Await();
	}

	int Http::Download(lua_State* lua)
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
			return 0;
		}
		if (!lua_isstring(lua, 3))
		{
			luaL_error(lua, "content must string");
			return 0;
		}
		size_t size = 0;
		const std::string url(lua_tolstring(lua, 2, &size));
		const std::string path(lua_tolstring(lua, 3, &size));

		std::string dir, fileName;
		if(Helper::Directory::GetDirAndFileName(path, dir, fileName))
		{
			if(!Helper::Directory::MakeDir(dir))
			{
				luaL_error(lua, "make dir error : %s", dir.c_str());
				return 0;
			}
		}

		HttpComponent* httpComponent = UserDataParameter::Read<HttpComponent*>(lua, 1);
		std::shared_ptr<HttpGetRequest> getRequest = HttpGetRequest::Create(url);
		if(getRequest == nullptr)
		{
			luaL_error(lua, "parse get url : [%s] failure", url.c_str());
			return 0;
		}

		std::fstream * fs = new std::fstream ();
		fs->open(path,  std::ios::out);
		if(!fs->is_open())
		{
			delete fs;
			luaL_error(lua, "open %s error", path.c_str());
			return 0;
		}

		TaskComponent* taskComponent = App::Get()->GetTaskComponent();
		std::shared_ptr<LuaWaitTaskSource> luaWaitTaskSource(new LuaWaitTaskSource(lua));
		taskComponent->Start([httpComponent, luaWaitTaskSource, getRequest, path, fs]()
		{
			std::shared_ptr<HttpRequestClient> requestClient = httpComponent->CreateClient();
			if(requestClient->Request(getRequest, fs) != nullptr)
			{
				luaWaitTaskSource->SetResult(XCode::Successful);
				return;
			}
			luaWaitTaskSource->SetResult(XCode::NetWorkError);
		});

		return luaWaitTaskSource->Await();
	}
}