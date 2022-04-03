#include"LuaScriptComponent.h"
#include"Script/luadebug.h"
#include"Script/luaExtension.h"
#include"Script/SystemExtension.h"
#include"Script/CoroutineExtension.h"

#include"App/App.h"
#include"Util/DirectoryHelper.h"
#include"Util/FileHelper.h"
#include"Util/MD5.h"
#include"Async/LuaServiceTaskSource.h"
#include"Component/Service/LuaRpcService.h"
#include"Async/LuaTaskSource.h"

using namespace Lua;
namespace Sentry
{
	bool LuaScriptComponent::Awake()
	{
		this->mLuaEnv = luaL_newstate();
		luaL_openlibs(mLuaEnv);
		return this->LoadAllFile();
	}

	bool LuaScriptComponent::LateAwake()
	{
		std::vector<Component *> components;
		this->GetComponents(components);

		for(Component * component : components)
		{
			ILuaRegister * luaRegister = component->Cast<ILuaRegister>();
			if(luaRegister != nullptr)
			{
				luaRegister->OnLuaRegister(this->mLuaEnv);
			}
		}

		if (Lua::lua_getfunction(this->mLuaEnv, "Main", "Awake"))
		{
			if (lua_pcall(this->mLuaEnv, 0, 0, 0) != 0)
			{
				LOG_ERROR(lua_tostring(this->mLuaEnv, -1));
				return false;
			}
			if (!(bool)lua_toboolean(this->mLuaEnv, -1))
			{
				return false;
			}
		}
		return true;
	}

	void LuaScriptComponent::OnLuaRegister(lua_State * lua)
	{
		Lua::ClassProxyHelper::BeginRegister<LuaTaskSource>(lua, "LuaTaskSource");
		Lua::ClassProxyHelper::PushCtor<LuaTaskSource>(lua);
		Lua::ClassProxyHelper::PushStaticExtensionFunction(lua, "LuaTaskSource", "SetResult", LuaTaskSource::SetResult);

		ClassProxyHelper::PushStaticFunction(lua, "Time", "GetDateStr", Helper::Time::GetDateStr);
		ClassProxyHelper::PushStaticFunction(lua, "Time", "GetDateString", Helper::Time::GetDateString);
		ClassProxyHelper::PushStaticFunction(lua, "Time", "GetNowSecTime", Helper::Time::GetNowSecTime);
		ClassProxyHelper::PushStaticFunction(lua, "Time", "GetNowMilTime", Helper::Time::GetNowMilTime);
		ClassProxyHelper::PushStaticFunction(lua, "Time", "GetYearMonthDayString",Helper::Time::GetYearMonthDayString);

	}

	void LuaScriptComponent::OnStart()
	{
		if(this->Call("Main", "Start")->Await<bool>())
		{

		}
	}

	bool LuaScriptComponent::LoadAllFile()
	{
		std::vector<std::string> luaPaths;
		std::vector<std::string> luaFiles;
		const ServerConfig& config = App::Get()->GetConfig();
		LOG_CHECK_RET_FALSE(config.GetMember("lua_src", luaPaths));
		for (const std::string& path : luaPaths)
		{
			if (!Helper::Directory::GetFilePaths(path, "*.lua", luaFiles))
			{
				LOG_ERROR("load", path, "lua file failure");
				return false;
			}
		}

		std::string dir, name, luaFile;
		for (std::string& path : luaFiles)
		{
			if (Helper::File::ReadTxtFile(path, luaFile)
				&& Helper::Directory::GetDirAndFileName(path, dir, name))
			{
				const std::string& md5 = Helper::Md5::GetMd5(luaFile);
				auto iter = this->mLuaFileMd5s.find(name);
				if (iter == this->mLuaFileMd5s.end())
				{
					mLuaFileMd5s.emplace(name, md5);
					LOG_CHECK_RET_FALSE(this->LoadLuaScript(path));
				}
				else if (iter->second != md5)
				{
					mLuaFileMd5s[name] = md5;
					LOG_CHECK_RET_FALSE(this->LoadLuaScript(path));
				}
			}
		}
		return true;
	}

	void LuaScriptComponent::OnDestory()
	{
		if (this->mLuaEnv != nullptr)
		{
			//lua_close(this->mLuaEnv);
		}
	}

	bool LuaScriptComponent::GetLuaTable(const std::string& name)
	{
		auto iter = this->mGlobalRefMap.find(name);
		if (iter != this->mGlobalRefMap.end())
		{
			int ref = iter->second;
			lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, ref);
			return (bool)lua_istable(this->mLuaEnv, -1);
		}
		lua_getglobal(this->mLuaEnv, name.c_str());
		if (lua_istable(this->mLuaEnv, -1))
		{
			int ref = luaL_ref(this->mLuaEnv, LUA_REGISTRYINDEX);
			this->mGlobalRefMap.emplace(name, ref);
			return true;
		}
		return false;
	}

	bool LuaScriptComponent::GetLuaFunction(const std::string& tab, const std::string& func)
	{
		if (!this->GetLuaTable(tab))
		{
			return false;
		}
		lua_getfield(this->mLuaEnv, -1, func.c_str());
		if (lua_isfunction(this->mLuaEnv, -1))
		{
			int ref = luaL_ref(this->mLuaEnv, LUA_REGISTRYINDEX);
			const std::string name = fmt::format("{0}.{1}", tab, func);
			this->mGlobalRefMap.emplace(name, ref);
			return true;
		}
		return false;
	}

	bool LuaScriptComponent::LoadLuaScript(const std::string filePath)
	{
		lua_pushcclosure(mLuaEnv, LuaDebug::onError, 0);
		int errfunc = lua_gettop(mLuaEnv);
		if (luaL_loadfile(mLuaEnv, filePath.c_str()) == 0)
		{
			lua_pcall(mLuaEnv, 0, 1, errfunc);
			lua_pop(mLuaEnv, 2);
			//LOG_DEBUG(fmt::format("load [{0}] successful", filePath));
			return true;
		}
		LOG_ERROR("load ", filePath, " failure : ", lua_tostring(mLuaEnv, -1));
		lua_pop(mLuaEnv, 1);
		return false;
	}

	void LuaScriptComponent::ClearRequirePath()
	{
		std::string path = "";
		lua_getglobal(mLuaEnv, "package");
		lua_pushlstring(mLuaEnv, path.c_str(), path.size());
		lua_setfield(mLuaEnv, -3, "path");
	}

	void LuaScriptComponent::AddRequirePath(const std::string path)
	{
		std::vector<std::string> luaFiles;
		if (Helper::Directory::GetFilePaths(path, "*.lua", luaFiles))
		{
			for (std::string& file : luaFiles)
			{
				this->LoadLuaScript(file);
			}
		}
		lua_getglobal(mLuaEnv, "package");
		lua_getfield(mLuaEnv, -1, "path");
		std::string nRequestPath = lua_tostring(mLuaEnv, -1);
		if (nRequestPath.find(path) != std::string::npos)
		{
			return;
		}
		char pathBuffer[4096] = { 0 };
#ifdef _MSC_VER
		size_t size = sprintf_s(pathBuffer, "%s;%s/?.lua", nRequestPath.c_str(), path.c_str());
#else
		size_t size = sprintf(pathBuffer, "%s;%s/?.lua", nRequestPath.c_str(), path.c_str());
#endif
		lua_pushlstring(mLuaEnv, pathBuffer, size);
		lua_setfield(mLuaEnv, -3, "path");
	}

	LuaTaskSource * LuaScriptComponent::Call(int ref)
	{
		if (!lua_getfunction(this->mLuaEnv, "coroutine", "call"))
		{
			return nullptr;
		}
		lua_rawgeti(this->mLuaEnv, LUA_REGISTRYINDEX, ref);
		if (!lua_isfunction(this->mLuaEnv, -1))
		{
			return nullptr;
		}
		if (lua_pcall(this->mLuaEnv, 1, 1, 0) != 0)
		{
			LOG_ERROR(lua_tostring(this->mLuaEnv, -1));
			return nullptr;
		}
		return PtrProxy<LuaTaskSource>::Read(this->mLuaEnv, -1);
	}

	LuaTaskSource* LuaScriptComponent::Call(const char* tab, const char* func)
	{
		if (!lua_getfunction(this->mLuaEnv, "coroutine", "call"))
		{
			return nullptr;
		}
		if(!lua_getfunction(this->mLuaEnv, tab, func))
		{
			return nullptr;
		}
		if (!lua_isfunction(this->mLuaEnv, -1))
		{
			return nullptr;
		}
		if (lua_pcall(this->mLuaEnv, 1, 1, 0) != 0)
		{
			LOG_ERROR(lua_tostring(this->mLuaEnv, -1));
			return nullptr;
		}
		return PtrProxy<LuaTaskSource>::Read(this->mLuaEnv, -1);
	}
}