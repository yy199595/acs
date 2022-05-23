#include"LuaScriptComponent.h"
#include"Script/luadebug.h"

#include"App/App.h"
#include"Util/DirectoryHelper.h"
#include"Util/FileHelper.h"
#include"Util/MD5.h"
#include"Async/LuaTaskSource.h"
#include"Async/LuaServiceTaskSource.h"
#include"Script/Extension/App/LuaApp.h"
#include"Script/Extension/Timer/Timer.h"
#include"Script/Extension/Log/LuaLogger.h"
#include"Script/Extension/Json/Json.h"
#include"Script/Extension/Coroutine/LuaCoroutine.h"

#include"Script/Extension/Bson/bson.h"
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
		for (const std::string& name : components)
		{
			ILuaRegister* luaRegisterComponent = this->GetApp()->GetComponent<ILuaRegister>(name);
			if (luaRegisterComponent != nullptr)
			{
				Lua::ClassProxyHelper luaRegister(this->mLuaEnv, name);
				luaRegisterComponent->OnLuaRegister(luaRegister);
			}
		}

		Lua::ClassProxyHelper luaRegister0(this->mLuaEnv, "App");
		luaRegister0.BeginNewTable();
		luaRegister0.PushExtensionFunction("GetComponent", Lua::LuaApp::GetComponent);

		Lua::ClassProxyHelper luaRegister1(this->mLuaEnv, "LuaTaskSource");
		luaRegister1.BeginRegister<LuaTaskSource>();
		luaRegister1.PushCtor<LuaTaskSource>();
		luaRegister1.PushStaticExtensionFunction("SetResult", LuaTaskSource::SetResult);

		Lua::ClassProxyHelper luaRegister2(this->mLuaEnv, "LuaServiceTaskSource");
		luaRegister2.BeginRegister<LuaServiceTaskSource>();
		luaRegister2.PushCtor<LuaServiceTaskSource>();
		luaRegister2.PushMemberFunction("SetError", &LuaServiceTaskSource::SetError);
		luaRegister2.PushMemberFunction("SetResult", &LuaServiceTaskSource::SetResult);

		Lua::ClassProxyHelper luaRegister3(this->mLuaEnv, "Time");
		luaRegister3.BeginNewTable();
		luaRegister3.PushStaticFunction("GetDateStr", Helper::Time::GetDateStr);
		luaRegister3.PushStaticFunction("GetDateString", Helper::Time::GetDateString);
		luaRegister3.PushStaticFunction("GetNowSecTime", Helper::Time::GetNowSecTime);
		luaRegister3.PushStaticFunction("GetNowMilTime", Helper::Time::GetNowMilTime);
		luaRegister3.PushStaticFunction("GetYearMonthDayString", Helper::Time::GetYearMonthDayString);

		Lua::ClassProxyHelper luaRegister4(this->mLuaEnv, "coroutine");
		luaRegister4.PushExtensionFunction("start", Lua::Coroutine::Start);
		luaRegister4.PushExtensionFunction("sleep", Lua::Coroutine::Sleep);

		Lua::ClassProxyHelper luaRegister5(this->mLuaEnv, "Log");
		luaRegister5.BeginNewTable();
		luaRegister5.PushExtensionFunction("Info", Lua::Log::DebugInfo);
		luaRegister5.PushExtensionFunction("Debug", Lua::Log::DebugLog);
		luaRegister5.PushExtensionFunction("Error", Lua::Log::DebugError);
		luaRegister5.PushExtensionFunction("Warning", Lua::Log::DebugWarning);

		Lua::ClassProxyHelper luaRegister6(this->mLuaEnv, "Timer");
		luaRegister6.BeginNewTable();
		luaRegister6.PushExtensionFunction("AddTimer", Lua::Timer::AddTimer);
		luaRegister6.PushExtensionFunction("CancelTimer", Lua::Timer::CancelTimer);

		Lua::ClassProxyHelper luaRegister7(this->mLuaEnv, "Json");
		luaRegister7.BeginNewTable();
		luaRegister7.PushExtensionFunction("Encode", Lua::Json::Encode);
		luaRegister7.PushExtensionFunction("Decode", Lua::Json::Decode);

//		Lua::ClassProxyHelper luaRegister8(this->mLuaEnv, "Bson");
//		luaRegister8.BeginNewTable();
//		luaRegister8.PushExtensionFunction("Encode", luabson::lencode);
//		luaRegister8.PushExtensionFunction("Data", luabson::ldate);
//		luaRegister8.PushExtensionFunction("Timestamp", luabson::ltimestamp);
//		luaRegister8.PushExtensionFunction("Regex", luabson::lregex);
//		luaRegister8.PushExtensionFunction("Objectid", luabson::lobjectid);
//		luaRegister8.PushExtensionFunction("Decode", luabson::ldecode);


		std::shared_ptr<Lua::Function> luaFunction = Lua::Function::Create(this->mLuaEnv, "Main", "Awake");
		if(luaFunction != nullptr)
		{
			luaFunction->Action();
		}
		return true;
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
		LOG_CHECK_RET_FALSE(config.GetMember("lua", luaPaths));
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