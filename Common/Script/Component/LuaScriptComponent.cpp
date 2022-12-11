#include"LuaScriptComponent.h"
#include"Lua/luadebug.h"

#include"App/App.h"
#include"Md5/MD5.h"
#include"Guid/Guid.h"
#include"File/DirectoryHelper.h"
#include"Lua/WaitLuaTaskSource.h"
#include"Lua/LuaServiceTaskSource.h"
#include"Lua/LuaApp.h"
#include"Lua/LuaLogger.h"
#include"Json/Lua/Json.h"
#include"Lua/LuaCoroutine.h"
#include"Service/LuaRpcService.h"
#include"Json/Lua/Encoder.h"
#include"Md5/LuaMd5.h"
#include"System/System.h"
#include"Config/ServiceConfig.h"
#include"Lua/LuaService.h"
#include"Component/TextConfigComponent.h"
using namespace Lua;
namespace Sentry
{
	bool LuaScriptComponent::Awake()
	{
		this->mLuaEnv = luaL_newstate();
		luaL_openlibs(mLuaEnv);
        return true;
	}

	bool LuaScriptComponent::LateAwake()
    {
        const ServerConfig *config = ServerConfig::Inst();
        const std::string &json = config->GetContent();
        values::pushDecoded(this->mLuaEnv, json.c_str(), json.size());
        lua_setglobal(this->mLuaEnv, "ServerConfig");

        Lua::ClassProxyHelper luaRegister(this->mLuaEnv, "ServerConfig");
        luaRegister.BeginNewTable();
        lua_getglobal(this->mLuaEnv, "ServerConfig");

        Lua::ClassProxyHelper luaRegister0(this->mLuaEnv, "App");
        luaRegister0.BeginRegister<App>();		
        luaRegister0.PushExtensionFunction("GetComponent", Lua::LuaApp::GetComponent);

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

        Lua::ClassProxyHelper luaRegister5(this->mLuaEnv, "Log");
        luaRegister5.BeginNewTable();
        luaRegister5.PushExtensionFunction("Info", Lua::Log::Info);
        luaRegister5.PushExtensionFunction("Debug", Lua::Log::Debug);
		luaRegister5.PushExtensionFunction("Error", Lua::Log::Error);
		luaRegister5.PushExtensionFunction("Warn", Lua::Log::Warning);
		luaRegister5.PushExtensionFunction("LuaError", Lua::Log::LuaError);

		Lua::ClassProxyHelper luaRegister55(this->mLuaEnv, "Console");
		luaRegister55.BeginNewTable();
		luaRegister55.PushExtensionFunction("Info", Lua::Console::Info);
		luaRegister55.PushExtensionFunction("Debug", Lua::Console::Debug);
		luaRegister55.PushExtensionFunction("Warn", Lua::Console::Warning);
		luaRegister55.PushExtensionFunction("Error", Lua::Console::Error);

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

		Lua::ClassProxyHelper luaRegister9(this->mLuaEnv, "Service");
		luaRegister9.BeginNewTable();
		luaRegister9.PushExtensionFunction("Call", Lua::Service::Call);
        luaRegister9.PushExtensionFunction("Send", Lua::Service::Send);
        luaRegister9.PushExtensionFunction("FindService", Lua::Service::FindService);
        luaRegister9.PushExtensionFunction("AllotServer", Lua::Service::AllotServer);
        luaRegister9.PushExtensionFunction("GetServerList", Lua::Service::GetServerList);


		std::vector<ILuaRegister *> components;
        this->mApp->GetComponents(components);
        for (ILuaRegister * component: components)
		{
			const std::string& name = dynamic_cast<Component*>(component)->GetName();
			Lua::ClassProxyHelper luaRegister(this->mLuaEnv, name);
			component->OnLuaRegister(luaRegister);
		}
        return this->LoadAllFile();
    }

    bool LuaScriptComponent::GetFunction(const std::string &tab, const std::string &func)
    {
        return Lua::Function::Get(this->mLuaEnv, tab.c_str(), func.c_str());
    }

    Lua::LuaModule * LuaScriptComponent::LoadModule(const std::string &name)
    {
		auto iter1 = this->mModules.find(name);
		if(iter1 != this->mModules.end())
		{
			return iter1->second.get();
		}
        auto iter = this->mModulePaths.find(name);
        if(iter == this->mModulePaths.end())
        {
            return nullptr;
        };
        const std::string & path = iter->second;
		std::unique_ptr<Lua::LuaModule> luaModule =
			std::make_unique<Lua::LuaModule>(this->mLuaEnv, name, path);
		Lua::LuaModule * result = luaModule.get();
		this->mModules.emplace(name, std::move(luaModule));
		//CONSOLE_LOG_INFO("load lua module [" << name << "] successful");
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

	bool LuaScriptComponent::Start()
	{
        const std::string& name = ServerConfig::Inst()->Name();
        Lua::LuaModule * luaModule = this->GetModule(name);
        return luaModule == nullptr || luaModule->Start();
	}

	void LuaScriptComponent::OnLocalComplete()
	{
		auto iter = this->mModules.begin();
		for(; iter != this->mModules.end(); iter++)
		{
			iter->second->OnLocalComplete();
		}
	}

	void LuaScriptComponent::OnClusterComplete()
	{
		auto iter = this->mModules.begin();
		for(; iter != this->mModules.end(); iter++)
		{
			iter->second->OnClusterComplete();
		}
	}

	bool LuaScriptComponent::LoadAllFile()
    {
        std::string common, module;
        std::vector<std::string> luaFiles;
        const ServerConfig *config = ServerConfig::Inst();
        LOG_CHECK_RET_FALSE(config->GetMember("lua", "common", common));
        LOG_CHECK_RET_FALSE(config->GetMember("lua", "module", module));

        common = System::FormatPath(common);
        this->mModulePath = System::FormatPath(module);
        if(!Helper::Directory::GetFilePaths(common, "*.lua", luaFiles))
        {
            return false;
        }
        for (const std::string &path: luaFiles)
        {
            if(luaL_dofile(this->mLuaEnv, path.c_str()) != LUA_OK)
            {
                LOG_ERROR(lua_tostring(this->mLuaEnv, -1));
                return false;
            }
        }
        this->LoadAllFilePath(this->mModulePath);
        
       
        this->AddRequire(common);
        this->AddRequire(this->mModulePath);       
        LuaModule * luaModule = this->LoadModule(ServerConfig::Inst()->Name());
        return luaModule == nullptr || luaModule->Awake();
    }

    bool LuaScriptComponent::LoadAllFilePath(const std::string & dir)
    {
        std::vector<std::string> luaFiles;
        std::string direct, name, luaFile;
        if (!Helper::Directory::GetFilePaths(dir, "*.lua", luaFiles))
        {
            return false;
        }
        for (const std::string& path : luaFiles)
        {
            Helper::Directory::GetDirAndFileName(path, direct, name);
            const std::string moduleName = name.substr(0, name.find('.'));
            {
                this->mModulePaths[moduleName] = path;
                //LOG_INFO("[" << moduleName << "] = <<" << path << ">>");
            }          
        }
        return true;
    }

    void LuaScriptComponent::OnHotFix()
    {
        this->LoadAllFilePath(this->mModulePath);
        std::vector<Component *> components;
        this->mApp->GetComponents(components);
        for (Component* component : components)
        {
            const std::string & name = component->GetName();
            IServiceBase * service = component->Cast<IServiceBase>();
            if (service != nullptr && this->GetModule(name) == nullptr)
            {
                Lua::LuaModule * luaModule = this->LoadModule(component->GetName());
                if(luaModule != nullptr && !luaModule->Awake())
                {
                    LOG_ERROR("load lua module [" << name << "failure");
                }
            }
        }
        
		auto iter = this->mModules.begin();
		for(; iter != this->mModules.end(); iter++)
		{
			const std::string & name = iter->first;
			Lua::LuaModule * luaModule = iter->second.get();
			if(luaModule != nullptr && luaModule->Hotfix())
			{
				LOG_INFO(name << " hotfix successful");
                IServiceBase * service = this->GetComponent<IServiceBase>(name);
                if (service != nullptr && service->IsStartService())
                {
                    service->LoadFromLua();
                }
			}
		}
    }

	void LuaScriptComponent::OnDestory()
	{
		if (this->mLuaEnv != nullptr)
		{
			//lua_close(this->mLuaEnv);
		}
	}

    void LuaScriptComponent::AddRequire(const std::string &path)
    {
        if(this->mDirectorys.find(path) == this->mDirectorys.end())
        {
            size_t size = 0;
            this->mDirectorys.insert(path);
            lua_getglobal(this->mLuaEnv, "package");
            lua_getfield(this->mLuaEnv, -1, "path");
            const char *str = lua_tolstring(this->mLuaEnv, -1, &size);
            std::string fullPath = std::string(str, size) + ";" + path + "/?.lua";
            lua_pushlstring(this->mLuaEnv, fullPath.c_str(), fullPath.size());
            lua_setfield(this->mLuaEnv, -3, "path");
        }
    }

	double LuaScriptComponent::GetMemorySize()
	{
		if(Lua::Function::Get(this->mLuaEnv, "collectgarbage"))
		{
			lua_pushstring(this->mLuaEnv, "count");
			if(lua_pcall(this->mLuaEnv, 1, 1, 0) != LUA_OK)
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
		document.Add("memory").Add(fmt::format("{0}mb", size / 1024));
	}
}