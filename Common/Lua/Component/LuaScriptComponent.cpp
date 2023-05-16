#include "LuaScriptComponent.h"

#include "Entity/Unit/App.h"
#include "Util/Md5/MD5.h"
#include "Util/Guid/Guid.h"
#include "Util/File/FileHelper.h"
#include "Util/File/DirectoryHelper.h"
#include "Async/Lua/WaitLuaTaskSource.h"
#include "Rpc//Lua/LuaServiceTaskSource.h"
#include "Entity/Lua/LuaApp.h"
#include "Log/Lua/LuaLogger.h"
#include "Util/Json/Lua/Json.h"
#include "Async/Lua/LuaCoroutine.h"
#include "Util/Md5/LuaMd5.h"
#include "Core/System/System.h"
#include "Server/Config/ServiceConfig.h"
#include "Util/Json/JsonWriter.h"
#include "Lua/Engine/Function.h"
#include "Lua/Module/LuaModule.h"
#include "Lua/Engine/ClassProxyHelper.h"
#include "Server/Component/TextConfigComponent.h"

using namespace Lua;
namespace Tendo
{
	bool LuaScriptComponent::Awake()
	{
		this->mLuaEnv = luaL_newstate();
		luaL_openlibs(mLuaEnv);
		return true;
	}

	bool LuaScriptComponent::LateAwake()
	{
		{
			Lua::ClassProxyHelper os(this->mLuaEnv, "os");

			os.PushMember("dir", System::WorkPath());
			os.PushStaticFunction("ms", Helper::Time::NowMilTime);
			os.PushStaticFunction("time", Helper::Time::NowSecTime);
#ifdef __OS_MAC__
			os.PushMember("type", std::string("mac"));
#elif __OS_LINUX__
			os.PushMember("type", std::string("linux"));
#elif __OS__WIN__
			os.PushMember("type", std::string("win"));
#endif

#ifdef __DEBUG__
			os.PushMember("debug", true);
#else
			os.PushMember("debug", false);
#endif
		}
		{
			Lua::ClassProxyHelper app(this->mLuaEnv, "App");
			app.BeginRegister<App>();
			app.PushExtensionFunction("Send", Lua::LuaApp::Send);
			app.PushExtensionFunction("Call", Lua::LuaApp::Call);
			app.PushExtensionFunction("GetAddr", Lua::LuaApp::GetAddr);
			app.PushExtensionFunction("GetComponent", Lua::LuaApp::GetComponent);
		}

		Lua::ClassProxyHelper luaRegister1(this->mLuaEnv, "WaitLuaTaskSource");
		luaRegister1.BeginRegister<WaitLuaTaskSource>();
		luaRegister1.PushCtor<WaitLuaTaskSource>();
		luaRegister1.PushStaticExtensionFunction("SetResult", WaitLuaTaskSource::SetResult);

		Lua::ClassProxyHelper luaRegister2(this->mLuaEnv, "LuaServiceTaskSource");
		luaRegister2.BeginRegister<LuaServiceTaskSource>();
		luaRegister2.PushExtensionFunction("SetRpc", LuaServiceTaskSource::SetRpc);
		luaRegister2.PushExtensionFunction("SetHttp", LuaServiceTaskSource::SetHttp);

		Lua::ClassProxyHelper luaRegister3(this->mLuaEnv, "Time");
		luaRegister3.BeginNewTable();
		luaRegister3.PushStaticFunction("GetDateStr", Helper::Time::GetDateStr);
		luaRegister3.PushStaticFunction("GetDateString", Helper::Time::GetDateString);
		luaRegister3.PushStaticFunction("NowSecTime", Helper::Time::NowSecTime);
		luaRegister3.PushStaticFunction("NowMilTime", Helper::Time::NowMilTime);
		luaRegister3.PushStaticFunction("GetYearMonthDayString", Helper::Time::GetYearMonthDayString);

		Lua::ClassProxyHelper luaRegister4(this->mLuaEnv, "coroutine");
		luaRegister4.PushExtensionFunction("start", Lua::Coroutine::Start);
		luaRegister4.PushExtensionFunction("sleep", Lua::Coroutine::Sleep);

		Lua::ClassProxyHelper luaRegister5(this->mLuaEnv, "Logger");
		luaRegister5.BeginNewTable();
		luaRegister5.PushExtensionFunction("Output", Lua::Log::Output);

		Lua::ClassProxyHelper luaRegister55(this->mLuaEnv, "ConsoleLog");
		luaRegister55.BeginNewTable();
		luaRegister55.PushExtensionFunction("Show", Lua::Console::Show);

		Lua::ClassProxyHelper luaRegister6(this->mLuaEnv, "Guid");
		luaRegister6.BeginNewTable();
		luaRegister6.PushStaticFunction("Create", Helper::Guid::Create);

		Lua::ClassProxyHelper luaRegister7(this->mLuaEnv, "rapidjson");
		luaRegister7.BeginNewTable();
		luaRegister7.PushExtensionFunction("encode", Lua::RapidJson::Encode);
		luaRegister7.PushExtensionFunction("decode", Lua::RapidJson::Decode);

		Lua::ClassProxyHelper luaRegister8(this->mLuaEnv, "Md5");
		luaRegister8.BeginNewTable();
		luaRegister8.PushExtensionFunction("ToString", Lua::Md5::ToString);

		std::vector<ILuaRegister*> components;
		this->mApp->GetComponents(components);
		for (ILuaRegister* component: components)
		{
			const std::string& name = dynamic_cast<Component*>(component)->GetName();
			Lua::ClassProxyHelper luaRegister(this->mLuaEnv, name);
			component->OnLuaRegister(luaRegister);
		}
		return this->LoadAllFile();
	}

	bool LuaScriptComponent::GetFunction(const std::string& tab, const std::string& func)
	{
		return Lua::Function::Get(this->mLuaEnv, tab.c_str(), func.c_str());
	}

	Lua::LuaModule* LuaScriptComponent::LoadModule(const std::string& name)
	{
		auto iter1 = this->mModules.find(name);
		if (iter1 != this->mModules.end())
		{
			return iter1->second.get();
		}
		const ServerConfig * config = ServerConfig::Inst();
		for(const std::string & path : config->RequirePath())
		{
			std::string fileName;
			std::vector<std::string> luaFiles;
			Helper::Directory::GetFilePaths(path, "*.lua", luaFiles);
			for (const std::string& filePath : luaFiles)
			{
				Helper::File::GetFileName(filePath, fileName);
				if (name == fileName)
				{
					return this->LoadModuleByPath(filePath);
				}
			}
		}
		return nullptr;
	}

	Lua::LuaModule* LuaScriptComponent::LoadModuleByPath(const std::string& path)
	{
		std::string name;
		Helper::File::GetFileName(path, name);
		std::unique_ptr<Lua::LuaModule> luaModule =
				std::make_unique<Lua::LuaModule>(this->mLuaEnv, name, path);
		if (!luaModule->Awake())
		{
			LOG_ERROR("load lua module [" << name << "] error");
			return nullptr;
		}
		Lua::LuaModule* result = luaModule.get();
		{
			LOG_INFO("start load lua module [" << name << "]");
			this->mModules.emplace(name, std::move(luaModule));
		}
		return result;
	}

	Lua::LuaModule* LuaScriptComponent::GetModule(const std::string& name)
	{
		auto iter1 = this->mModules.find(name);
		return iter1 == this->mModules.end() ? nullptr : iter1->second.get();
	}

	void LuaScriptComponent::OnSecondUpdate(const int tick)
	{
		auto iter = this->mModules.begin();
		for (; iter != this->mModules.end(); iter++)
		{
			iter->second->Update(tick);
		}
	}

	void LuaScriptComponent::Start()
	{
		const std::string& name = ServerConfig::Inst()->Name();
		Lua::LuaModule* luaModule = this->GetModule(name);
		if(luaModule != nullptr)
		{
			luaModule->Start();
		}
	}

	void LuaScriptComponent::OnComplete()
	{
		auto iter = this->mModules.begin();
		for (; iter != this->mModules.end(); iter++)
		{
			iter->second->OnComplete();
		}
	}

	bool LuaScriptComponent::LoadAllFile()
	{
		const ServerConfig* config = ServerConfig::Inst();
		for(const std::string & path : config->RequirePath())
		{
			this->AddRequire(path);
		}
		if (config->MainLua().empty())
		{
			return true;
		}
		std::string main(config->MainLua());
		return this->LoadModuleByPath(main) != nullptr;
	}

	void LuaScriptComponent::OnHotFix()
	{
		std::vector<Component*> components;
		this->mApp->GetComponents(components);
		for (Component* component: components)
		{
			const std::string& name = component->GetName();
			IServiceBase* service = component->Cast<IServiceBase>();
			if (service != nullptr && this->GetModule(name) == nullptr)
			{
				if (!this->LoadModule(component->GetName()))
				{
					LOG_ERROR("load lua module [" << name << "failure");
				}
			}
		}

		auto iter = this->mModules.begin();
		for (; iter != this->mModules.end(); iter++)
		{
			const std::string& name = iter->first;
			Lua::LuaModule* luaModule = iter->second.get();
			if (luaModule != nullptr && luaModule->Hotfix())
			{
				LOG_INFO(name << " hotfix successful");
				IServiceBase* service = this->GetComponent<IServiceBase>(name);
				if (service != nullptr && service->IsStartService())
				{
					service->LoadFromLua();
				}
			}
		}
	}

	void LuaScriptComponent::OnDestroy()
	{
		if (this->mLuaEnv != nullptr)
		{
			lua_close(this->mLuaEnv);
			this->mLuaEnv = nullptr;
		}
	}

	void LuaScriptComponent::AddRequire(const std::string& path)
	{
		if (path.empty())
			return;
		if (this->mDirectorys.find(path) == this->mDirectorys.end())
		{
			size_t size = 0;
			this->mDirectorys.insert(path);
			lua_getglobal(this->mLuaEnv, "package");
			lua_getfield(this->mLuaEnv, -1, "path");
			const char* str = lua_tolstring(this->mLuaEnv, -1, &size);
			std::string fullPath = std::string(str, size) + ";" + path + "/?.lua";
			lua_pushlstring(this->mLuaEnv, fullPath.c_str(), fullPath.size());
			lua_setfield(this->mLuaEnv, -3, "path");
		}
	}

	double LuaScriptComponent::CollectGarbage()
	{
		double start = this->GetMemorySize();
		if (Lua::Function::Get(this->mLuaEnv, "collectgarbage"))
		{
			lua_pushstring(this->mLuaEnv, "collect");
			if (lua_pcall(this->mLuaEnv, 1, 1, 0) != LUA_OK)
			{
				LOG_ERROR(lua_tostring(this->mLuaEnv, -1));
				return 0;
			}
		}
		return start - this->GetMemorySize();
	}

	double LuaScriptComponent::GetMemorySize()
	{
		if (Lua::Function::Get(this->mLuaEnv, "collectgarbage"))
		{
			lua_pushstring(this->mLuaEnv, "count");
			if (lua_pcall(this->mLuaEnv, 1, 1, 0) != LUA_OK)
			{
				LOG_ERROR(lua_tostring(this->mLuaEnv, -1));
				return 0;
			}
			return lua_tonumber(this->mLuaEnv, -1);
		}
		return 0;
	}

	void LuaScriptComponent::OnRecord(Json::Writer& document)
	{
		double size = this->GetMemorySize();
		double free = this->CollectGarbage();
		document.Add("free").Add(fmt::format("{0}mb", free / 1024));
		document.Add("memory").Add(fmt::format("{0}mb", size / 1024));
	}

	bool LuaScriptComponent::UnloadModule(const string& name)
	{
		auto iter = this->mModules.find(name);
		if (iter != this->mModules.end())
		{
			iter->second->Close();
			this->mModules.erase(iter);
			return true;
		}
		return false;
	}
}