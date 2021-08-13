#include"ScriptManager.h"
#include<Script/luadebug.h>
#include<Manager/NetSessionManager.h>
#include<Script/luaExtension.h>
#include<Script/SystemExtension.h>
#include<Script/LuaProtocExtension.h>
#include<Script/CoroutineExtension.h>

#include<Util/DirectoryHelper.h>

namespace Sentry
{
    ScriptManager::ScriptManager()
    {
        this->mLuaEnv = nullptr;
        this->mMainLuaTable = nullptr;
    }

    bool ScriptManager::OnInit()
    {
        if (this->mLuaEnv == nullptr)
        {
            this->mLuaEnv = luaL_newstate();
            luaL_openlibs(mLuaEnv);
        }
        SayNoAssertRetFalse_F(this->GetConfig().GetValue("Script","main", this->mMainLuaPath));
        SayNoAssertRetFalse_F(this->GetConfig().GetValue("Script","include",this->mRequirePaths));
        for (std::string &include : this->mRequirePaths)
        {
            this->AddRequirePath(include);
        }

        this->ForeachManagers([this](Manager *pManager) -> bool {
            pManager->PushClassToLua(this->mLuaEnv);
            return true;
        });

        this->OnPushGlobalObject();
        this->RegisterExtension(mLuaEnv);
        return this->LoadLuaScript(this->mMainLuaPath);
    }

    void ScriptManager::OnDestory()
    {
        if (this->mLuaEnv != nullptr)
        {
            lua_close(this->mLuaEnv);
        }
    }

    void ScriptManager::OnInitComplete()
    {

    }

    int ScriptManager::GetGlobalReference(const std::string &name)
    {
        auto iter = this->mGlobalRefMap.find(name);
        if (iter == this->mGlobalRefMap.end())
        {
            size_t pos = name.find(".");
            if (pos != std::string::npos)
            {
                const std::string tab = name.substr(0, pos);
                const std::string key = name.substr(pos + 1);
                lua_getglobal(this->mLuaEnv, tab.c_str());
                if (!lua_istable(this->mLuaEnv, -1))
                {
                    SayNoDebugError("find lua object fail " << name);
                    return 0;
                }
                lua_getfield(this->mLuaEnv, -1, key.c_str());
                if (lua_isnil(this->mLuaEnv, -1))
                {
                    SayNoDebugError("find lua object field fail " << name);
                    return 0;
                }
                int ref = luaL_ref(this->mLuaEnv, LUA_REGISTRYINDEX);
                this->mGlobalRefMap.emplace(name, ref);
                return ref;
            }
            lua_getglobal(this->mLuaEnv, name.c_str());
            if (lua_isnil(this->mLuaEnv, -1))
            {
                SayNoDebugError("find lua object field fail " << name);
                return 0;
            }
            int ref = luaL_ref(this->mLuaEnv, LUA_REGISTRYINDEX);
            this->mGlobalRefMap.emplace(name, ref);
        }
        return iter->second;
    }


    bool ScriptManager::LoadLuaScript(const std::string filePath)
    {
        lua_pushcclosure(mLuaEnv, LuaDebug::onError, 0);
        int errfunc = lua_gettop(mLuaEnv);
        if (luaL_loadfile(mLuaEnv, filePath.c_str()) == 0)
        {
            lua_pcall(mLuaEnv, 0, 1, errfunc);
            SayNoDebugLog("load lua script success path :" << filePath);
            lua_pop(mLuaEnv, 2);
            return this->LoadAllModule();
        }
        SayNoDebugError(lua_tostring(mLuaEnv, -1));
        lua_pop(mLuaEnv, 1);
        return false;
    }

    bool ScriptManager::LoadAllModule()
    {
        if (mMainLuaTable != nullptr)
        {
            delete mMainLuaTable;
            mMainLuaTable = nullptr;
        }
        mMainLuaTable = LuaTable::Create(mLuaEnv, "Main");
        if (mMainLuaTable != nullptr)
        {
            if (!mMainLuaTable->Action("Load"))
            {
                return false;
            }
            mMainLuaTable->Action("Start");
        }
        return true;
    }

    void ScriptManager::ClearRequirePath()
    {
        std::string path = "";
        lua_getglobal(mLuaEnv, "package");
        lua_pushlstring(mLuaEnv, path.c_str(), path.size());
        lua_setfield(mLuaEnv, -3, "path");
    }

    void ScriptManager::AddRequirePath(const std::string path)
    {
        lua_getglobal(mLuaEnv, "package");
        lua_getfield(mLuaEnv, -1, "path");
        std::string nRequestPath = lua_tostring(mLuaEnv, -1);
        if (nRequestPath.find(path) != std::string::npos)
        {
            return;
        }
        char pathBuffer[4096] = {0};
#ifdef _MSC_VER
        size_t size = sprintf_s(pathBuffer, "%s;%s/?.lua", nRequestPath.c_str(), path.c_str());
#else
        size_t size = sprintf(pathBuffer, "%s;%s/?.lua", nRequestPath.c_str(), path.c_str());
#endif
        lua_pushlstring(mLuaEnv, pathBuffer, size);
        lua_setfield(mLuaEnv, -3, "path");
    }

    void ScriptManager::PushClassToLua(lua_State *lua)
    {
        ClassProxyHelper::BeginRegister<Applocation>(lua, "Applocation");
        ClassProxyHelper::PushMemberFunction<Applocation>(lua, "GetRunTime", &Applocation::GetRunTime);
        ClassProxyHelper::PushMemberFunction<Applocation>(lua, "GetDelaTime", &Applocation::GetDelaTime);
        ClassProxyHelper::PushMemberFunction<Applocation>(lua, "GetLogicTime", &Applocation::GetLogicTime);

        ClassProxyHelper::BeginRegister<TcpClientSession>(lua, "TcpClientSession");
        ClassProxyHelper::PushMemberFunction<TcpClientSession>(lua, "GetIP", &TcpClientSession::GetIP);
        ClassProxyHelper::PushMemberFunction<TcpClientSession>(lua, "GetPort", &TcpClientSession::GetPort);
        ClassProxyHelper::PushMemberFunction<TcpClientSession>(lua, "GetAddress", &TcpClientSession::GetAddress);
        ClassProxyHelper::PushMemberFunction<TcpClientSession>(lua, "StartConnect", &TcpClientSession::StartConnect);

        ClassProxyHelper::PushStaticFunction(lua, "TimeHelper", "GetDateStr", TimeHelper::GetDateStr);
        ClassProxyHelper::PushStaticFunction(lua, "TimeHelper", "GetDateString", TimeHelper::GetDateString);
        ClassProxyHelper::PushStaticFunction(lua, "TimeHelper", "GetSecTimeStamp", TimeHelper::GetSecTimeStamp);
        ClassProxyHelper::PushStaticFunction(lua, "TimeHelper", "GetMilTimestamp", TimeHelper::GetMilTimestamp);
        ClassProxyHelper::PushStaticFunction(lua, "TimeHelper", "GetMicTimeStamp", TimeHelper::GetMicTimeStamp);
        ClassProxyHelper::PushStaticFunction(lua, "TimeHelper", "GetYearMonthDayString",
                                             TimeHelper::GetYearMonthDayString);

        ClassProxyHelper::PushStaticExtensionFunction(lua, "Sentry", "Call", SystemExtension::Call);
        ClassProxyHelper::PushStaticExtensionFunction(lua, "Sentry", "Sleep", SystemExtension::Sleep);
        ClassProxyHelper::PushStaticExtensionFunction(lua, "Sentry", "CallWait", SystemExtension::CallWait);
        ClassProxyHelper::PushStaticExtensionFunction(lua, "Sentry", "CallByName", SystemExtension::CallByName);
        ClassProxyHelper::PushStaticExtensionFunction(lua, "Sentry", "CallBySession", SystemExtension::CallBySession);

        ClassProxyHelper::PushStaticExtensionFunction(lua, "Sentry", "AddTimer", SystemExtension::AddTimer);
        ClassProxyHelper::PushStaticExtensionFunction(lua, "Sentry", "RemoveTimer", SystemExtension::RemoveTimer);

        ClassProxyHelper::PushStaticExtensionFunction(lua, "Sentry", "GetManager", SystemExtension::GetManager);
        ClassProxyHelper::PushStaticExtensionFunction(lua, "Sentry", "CreateByTable",
                                                      LuaProtocExtension::CreateByTable);

        ClassProxyHelper::PushStaticExtensionFunction(lua, "Sentry", "LuaRetMessage", SystemExtension::LuaRetMessage);
        ClassProxyHelper::PushStaticExtensionFunction(lua, "Sentry", "NewService", SystemExtension::NewService);

    }

    void ScriptManager::OnPushGlobalObject()
    {

    }

    bool ScriptManager::StartLoadScript()
    {
        std::vector<std::string> nAllLuaFile;
        if (!DirectoryHelper::GetFilePaths("Script", "*.lua", nAllLuaFile) || nAllLuaFile.empty())
        {
            SayNoDebugError("not find field or director");
            return false;
        }
        std::string nMainLau;
        std::string nFileName;
        std::string nLuaFileDir;
        for (std::string &path : nAllLuaFile)
        {
            if (DirectoryHelper::GetDirAndFileName(path, nLuaFileDir, nFileName))
            {
                this->AddRequirePath(nLuaFileDir);
                if (path.find("Main.lua") != std::string::npos)
                {
                    nMainLau = path;
                }
            }
        }
        if (!this->LoadLuaScript(nMainLau))
        {
            return false;
        }
        return this->LoadAllModule();
    }

    void ScriptManager::RegisterExtension(lua_State *lua)
    {
        ClassProxyHelper::PushStaticExtensionFunction(lua, "Sentry", "Log", LuaAPIExtension::DebugLog);
        ClassProxyHelper::PushStaticExtensionFunction(lua, "Sentry", "Info", LuaAPIExtension::DebugInfo);
        ClassProxyHelper::PushStaticExtensionFunction(lua, "Sentry", "Error", LuaAPIExtension::DebugError);
        ClassProxyHelper::PushStaticExtensionFunction(lua, "Sentry", "Warning", LuaAPIExtension::DebugWarning);


        lua_newtable(lua);
        lua_pushtablefunction(lua, "Serialization", LuaProtocExtension::Serialization);
        lua_setglobal(lua, "ProtocUtil");

        lua_getglobal(lua, "coroutine");
        lua_pushtablefunction(lua, "sleep", CoroutineExtension::Sleep);
        lua_pushtablefunction(lua, "start", CoroutineExtension::Start);


    }
}