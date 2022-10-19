#include"LuaScriptComponent.h"
#include"Lua/luadebug.h"

#include"App/App.h"
#include"Md5/MD5.h"
#include"Guid/Guid.h"
#include"File/DirectoryHelper.h"
#include"Lua/WaitLuaTaskSource.h"
#include"Lua/LuaServiceTaskSource.h"
#include"App/Lua/LuaApp.h"
#include"Lua/LuaLogger.h"
#include"Json/Lua/Json.h"
#include"Lua/LuaCoroutine.h"
#include"Service/RpcService.h"
#include"Json/Lua/Encoder.h"
#include"Md5/LuaMd5.h"
#include"App/System/System.h"
#include"Config/ClusterConfig.h"
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
        std::vector<std::string> components;
        this->mApp->GetComponents(components);
        for (const std::string &name: components)
        {
            ILuaRegister *luaRegisterComponent = this->GetComponent<ILuaRegister>(name);
            if (luaRegisterComponent != nullptr)
            {
                Lua::ClassProxyHelper luaRegister(this->mLuaEnv, name);
                luaRegisterComponent->OnLuaRegister(luaRegister);
            }
        }
        const ServerConfig *config = ServerConfig::Inst();
        const std::string &json = config->GetContent();
        values::pushDecoded(this->mLuaEnv, json.c_str(), json.size());
        lua_setglobal(this->mLuaEnv, "ServerConfig");

        Lua::ClassProxyHelper luaRegister(this->mLuaEnv, "ServerConfig");
        luaRegister.BeginNewTable();
        lua_getglobal(this->mLuaEnv, "ServerConfig");

        Lua::ClassProxyHelper luaRegister0(this->mLuaEnv, "App");
        luaRegister0.BeginRegister<App>();
        luaRegister0.PushExtensionFunction("GetService", Lua::LuaApp::GetService);
        luaRegister0.PushExtensionFunction("GetComponent", Lua::LuaApp::GetComponent);

        Lua::ClassProxyHelper luaRegister1(this->mLuaEnv, "WaitLuaTaskSource");
        luaRegister1.BeginRegister<WaitLuaTaskSource>();
        luaRegister1.PushCtor<WaitLuaTaskSource>();
        luaRegister1.PushStaticExtensionFunction("SetResult", WaitLuaTaskSource::SetResult);

        Lua::ClassProxyHelper luaRegister2(this->mLuaEnv, "LuaServiceTaskSource");
        luaRegister2.BeginRegister<LuaServiceTaskSource>();
        luaRegister2.PushExtensionFunction("SetResult", LuaServiceTaskSource::SetResult);

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

        Lua::ClassProxyHelper luaRegister6(this->mLuaEnv, "Guid");
        luaRegister6.BeginNewTable();
        luaRegister6.PushStaticFunction("Create", Helper::Guid::Create);

        Lua::ClassProxyHelper luaRegister7(this->mLuaEnv, "Json");
        luaRegister7.BeginNewTable();
        luaRegister7.PushExtensionFunction("Encode", Lua::Json::Encode);
        luaRegister7.PushExtensionFunction("Decode", Lua::Json::Decode);

        Lua::ClassProxyHelper luaRegister8(this->mLuaEnv, "Md5");
        luaRegister8.BeginNewTable();
        luaRegister8.PushExtensionFunction("ToString", Lua::Md5::ToString);

        LOG_CHECK_RET_FALSE(this->LoadAllFile());
        for (const std::string &module: this->mModules)
        {
            if (lua_getfunction(this->mLuaEnv, module.c_str(), "Awake"))
            {
                if (lua_pcall(this->mLuaEnv, 0, 0, 0) != LUA_OK)
                {
                    LOG_ERROR(lua_tostring(this->mLuaEnv, -1));
                    return false;
                }
            }
        }
        if (this->mMainTable && this->mMainTable->GetFunction("Awake"))
        {
            if (lua_pcall(this->mLuaEnv, 0, 0, 0) != LUA_OK)
            {
                LOG_ERROR(lua_tostring(this->mLuaEnv, -1));
                return false;
            }
        }
        return true;
    }

	bool LuaScriptComponent::Start()
	{
        if(this->mMainTable && this->mMainTable->GetFunction("Start"))
        {
            WaitLuaTaskSource * luaTaskSource = Lua::Function::Call(this->mLuaEnv);
            if(luaTaskSource != nullptr)
            {
                return luaTaskSource->Await<bool>();
            }
        }
		return true;
	}

	void LuaScriptComponent::OnLocalComplete()
	{
        if(this->mMainTable && this->mMainTable->GetFunction("OnLocalComplete"))
        {
            WaitLuaTaskSource * luaTaskSource = Lua::Function::Call(this->mLuaEnv);
            if(luaTaskSource != nullptr)
            {
                luaTaskSource->Await<void>();
            }
        }
	}

	void LuaScriptComponent::OnClusterComplete()
	{
        if(this->mMainTable && this->mMainTable->GetFunction("OnClusterComplete"))
        {
            WaitLuaTaskSource * luaTaskSource = Lua::Function::Call(this->mLuaEnv);
            if(luaTaskSource != nullptr)
            {
                luaTaskSource->Await<void>();
            }
        }
	}

	bool LuaScriptComponent::LoadAllFile()
	{
		std::vector<std::string> luaPaths;
		std::vector<std::string> luaFiles;
		const ServerConfig * config = ServerConfig::Inst();
        LOG_CHECK_RET_FALSE(config->GetMember("lua", "require", luaPaths));
        TextConfigComponent * textComponent = this->GetComponent<TextConfigComponent>();
        const ClusterConfig * clusterConfig = textComponent->GetTextConfig<ClusterConfig>();

        const std::string & lua = clusterConfig->GetConfig()->GetMainLua();
        if(!lua.empty())
        {
            this->mMainTable = std::make_shared<Lua::LocalTable>(this->mLuaEnv);
            if (!this->mMainTable->Load(System::GetWorkPath() + lua))
            {
                LOG_ERROR("load main lua : " << System::GetWorkPath() << lua);
                return false;
            }
        }
        std::string index;
        if(config->GetMember("lua", "http", index))
        {
            this->AddRequire(System::GetWorkPath() + index);
        }
		for (const std::string& path : luaPaths)
		{
			const std::string fullPath = System::GetWorkPath() + path;
			if (!Helper::Directory::GetFilePaths(fullPath, "*.lua", luaFiles))
			{
				LOG_ERROR("load" << path << " lua file failure");
				return false;
			}
		}
        std::string dir, name, luaFile;
        Lua::Function::Clear(this->mLuaEnv);
		for (const std::string& path : luaFiles)
		{
            std::ifstream is(path);
            if(!is.is_open())
            {
                LOG_ERROR("read lua file error : " << path);
                return false;
            }
            MD5 fileMd5(is);
			if (Helper::Directory::GetDirAndFileName(path, dir, name))
			{
				const std::string md5 = fileMd5.toString();
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

    void LuaScriptComponent::OnHotFix()
    {
        Lua::Function::Clear(this->mLuaEnv);
        if(this->LoadAllFile())
        {
            for(const std::string & module : this->mModules)
            {
                if(lua_getfunction(this->mLuaEnv, module.c_str(), "Hotfix"))
                {
                    if (lua_pcall(this->mLuaEnv, 0, 0, 0) != LUA_OK)
                    {
                        LOG_ERROR(lua_tostring(this->mLuaEnv, -1));
                        return;
                    }
                }
            }
            if(this->mMainTable && this->mMainTable->GetFunction("Hotfix"))
            {
                if (lua_pcall(this->mLuaEnv, 0, 0, 0) != LUA_OK)
                {
                    LOG_ERROR(lua_tostring(this->mLuaEnv, -1));
                    return;
                }
            }
            LOG_INFO("start hotfix successful");
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
        if (this->mDirectorys.find(path) == this->mDirectorys.end())
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

	bool LuaScriptComponent::LoadLuaScript(const std::string filePath)
    {
        std::string directory, moduleName;
        if (!Helper::Directory::GetDirAndFileName(filePath, directory, moduleName))
        {
            return false;
        }
        this->AddRequire(directory);
        moduleName = moduleName.substr(0, moduleName.find('.'));
        if (this->mMainTable && this->mMainTable->GetFunction("OnLoadModule"))
        {
            lua_pushstring(this->mLuaEnv, moduleName.c_str());
            if (lua_pcall(this->mLuaEnv, 1, 0, 0) != LUA_OK)
            {
                LOG_ERROR(lua_tostring(this->mLuaEnv, -1));
                return false;
            }
        }
        else
        {
            if(luaL_dofile(this->mLuaEnv, filePath.c_str()) != LUA_OK)
            {
                LOG_ERROR(lua_tostring(this->mLuaEnv, -1));
                return false;
            }
        }
        this->mModules.insert(moduleName);
        return true;
    }
}