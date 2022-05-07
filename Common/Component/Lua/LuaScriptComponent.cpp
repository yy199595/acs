#include"LuaScriptComponent.h"
#include"Script/luadebug.h"

#include"App/App.h"
#include"Util/DirectoryHelper.h"
#include"Util/FileHelper.h"
#include"Util/MD5.h"
#include"Async/LuaTaskSource.h"

using namespace Lua;
namespace Sentry
{
	void LuaScriptComponent::Awake()
	{
		this->mLuaEnv = luaL_newstate();
		luaL_openlibs(mLuaEnv);
		LOG_CHECK_RET(this->LoadAllFile());
	}

	bool LuaScriptComponent::LateAwake()
	{
		std::vector<std::string> components;
		this->GetApp()->GetComponents(components);
		for(const std::string & name : components)
		{
			ILuaRegister* luaRegister = this->GetApp()->GetComponent<ILuaRegister>(name);
			if(luaRegister != nullptr)
			{
				luaRegister->OnLuaRegister(this->mLuaEnv);
			}
		}
		std::shared_ptr<Lua::Function> luaFunction = Lua::Function::Create(this->mLuaEnv, "Main", "Awake");
		return luaFunction != nullptr && luaFunction->Func<bool>();
	}

	void LuaScriptComponent::OnLuaRegister(lua_State * lua)
	{
		Lua::ClassProxyHelper::BeginRegister<LuaTaskSource>(lua, "LuaTaskSource");
		Lua::ClassProxyHelper::PushCtor<LuaTaskSource>(lua);
		Lua::ClassProxyHelper::PushStaticExtensionFunction(lua, "LuaTaskSource", "SetResult", LuaTaskSource::SetResult);

		ClassProxyHelper::BeginNewTable(lua, "Time");
		ClassProxyHelper::PushStaticFunction(lua, "Time", "GetDateStr", Helper::Time::GetDateStr);
		ClassProxyHelper::PushStaticFunction(lua, "Time", "GetDateString", Helper::Time::GetDateString);
		ClassProxyHelper::PushStaticFunction(lua, "Time", "GetNowSecTime", Helper::Time::GetNowSecTime);
		ClassProxyHelper::PushStaticFunction(lua, "Time", "GetNowMilTime", Helper::Time::GetNowMilTime);
		ClassProxyHelper::PushStaticFunction(lua, "Time", "GetYearMonthDayString",Helper::Time::GetYearMonthDayString);

	}

	bool LuaScriptComponent::OnStart()
	{
		LuaTaskSource* luaTaskSource = Lua::Function::Call(this->mLuaEnv, "Main", "Start");
		return luaTaskSource == nullptr || luaTaskSource->Await<bool>();
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
				LOG_ERROR("load" << path << " lua file failure");
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

	bool LuaScriptComponent::LoadLuaScript(const std::string filePath)
	{
		lua_pushcclosure(mLuaEnv, LuaDebug::onError, 0);
		int top = lua_gettop(mLuaEnv);
		if (luaL_loadfile(mLuaEnv, filePath.c_str()) == 0)
		{
			lua_pcall(mLuaEnv, 0, 1, top);
			lua_pop(mLuaEnv, 2);
			//LOG_DEBUG(fmt::format("load [{0}] successful", filePath));
			return true;
		}
		LOG_ERROR("load "<< filePath << " failure : " << lua_tostring(mLuaEnv, -1));
		lua_pop(mLuaEnv, 1);
		return false;
	}
}