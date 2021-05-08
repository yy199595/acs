#include"ScriptManager.h"
#include<Core/Applocation.h>
#include<Script/Util/luadebug.h>
#include<Object/GameObject.h>
#include<Manager/ActionRegisterManager.h>
#include<Manager/NetWorkManager.h>
#include<Manager/TimerManager.h>

#include<NetWork/TcpClientSession.h>
#include<NetWork/RemoteScheduler.h>
#include<Script/luaExtension/luaExtension.h>
#include<Script/luaExtension/SystemExtension.h>
#include<Script/luaExtension/LuaProtocExtension.h>
#include<Script/luaExtension/CoroutineExtension.h>

#include<Util/JsonHelper.h>
#include<Util/FileHelper.h>
#include<Util/MathHelper.h>
#include<Util/ProtocHelper.h>
#include<Util/DirectoryHelper.h>
namespace SoEasy
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
		if (!this->GetConfig().GetValue("ScriptMain", this->mMainLuaPath))
		{
			SayNoDebugFatal("not find field 'ScriptMain'");
			return false;
		}
		if (!this->GetConfig().GetValue("ScriptInclude", this->mRequirePaths))
		{
			SayNoDebugFatal("not find field 'ScriptInclude'");
			return false;
		}
		for (std::string & include : this->mRequirePaths)
		{
			this->AddRequirePath(include);
		}

		this->ForeachManagers([this](Manager * pManager)->bool
		{
			pManager->PushClassToLua(this->mLuaEnv);
			return true;
		});

		this->OnPushGlobalObject();
		this->RegisterExtension(mLuaEnv);
		REGISTER_FUNCTION_0(ScriptManager::OnHotfix);
		return this->LoadLuaScript(this->mMainLuaPath);
	}

	void ScriptManager::OnDestory()
	{
		if (this->mLuaEnv != nullptr)
		{
			lua_close(this->mLuaEnv);
		}
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

	void ScriptManager::AddRequirePath(const std::string path)
	{
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

	void ScriptManager::PushClassToLua(lua_State * lua)
	{
		ClassProxyHelper::BeginNewTalbe(lua, "SoEasy");
		ClassProxyHelper::BeginRegister<ServerConfig>(lua, "ServerConfig");
		ClassProxyHelper::PushMemberFunction<ServerConfig>(lua, "GetFps", &ServerConfig::GetFps);
		ClassProxyHelper::PushMemberFunction<ServerConfig>(lua, "GetServerID", &ServerConfig::GetServerID);
		ClassProxyHelper::PushMemberFunction<ServerConfig>(lua, "GetServerIP", &ServerConfig::GetServerIP);
		ClassProxyHelper::PushMemberFunction<ServerConfig>(lua, "GetServerName", &ServerConfig::GetServerName);
		ClassProxyHelper::PushMemberFunction<ServerConfig>(lua, "GetServerPort", &ServerConfig::GetServerPort);
		ClassProxyHelper::PushMemberFunction<ServerConfig>(lua, "GetServerAreaID", &ServerConfig::GetServerAreaID);
		ClassProxyHelper::PushMemberFunction<ServerConfig>(lua, "GetCenterServerIP", &ServerConfig::GetCenterServerIP);
		ClassProxyHelper::PushMemberFunction<ServerConfig>(lua, "GetCenterServerPort", &ServerConfig::GetCenterServerPort);

		ClassProxyHelper::BeginRegister<Applocation>(lua, "Applocation");
		ClassProxyHelper::PushMemberFunction<Applocation>(lua, "GetRunTime", &Applocation::GetRunTime);
		ClassProxyHelper::PushMemberFunction<Applocation>(lua, "GetDelaTime", &Applocation::GetDelaTime);
		ClassProxyHelper::PushMemberFunction<Applocation>(lua, "GetLogicTime", &Applocation::GetLogicTime);

		ClassProxyHelper::BeginRegister<NetWorkManager>(lua, "NetWorkManager");
		ClassProxyHelper::PushMemberFunction<NetWorkManager>(lua, "GetSessionByAdress", &NetWorkManager::GetSessionByAdress);

		ClassProxyHelper::BeginRegister<TcpClientSession>(lua, "TcpClientSession");
		ClassProxyHelper::PushMemberFunction<TcpClientSession>(lua, "GetIP", &TcpClientSession::GetIP);
		ClassProxyHelper::PushMemberFunction<TcpClientSession>(lua, "GetPort", &TcpClientSession::GetPort);
		ClassProxyHelper::PushMemberFunction<TcpClientSession>(lua, "GetAddress", &TcpClientSession::GetAddress);
		ClassProxyHelper::PushMemberFunction<TcpClientSession>(lua, "StartConnect", &TcpClientSession::StartConnect);
		
		ClassProxyHelper::BeginRegister<ActionManager>(lua, "ActionManager");

		ClassProxyHelper::BeginNewTalbe(lua, "TimeHelper");
		ClassProxyHelper::PushStaticFunction(lua, "TimeHelper", "GetDateStr", TimeHelper::GetDateStr);
		ClassProxyHelper::PushStaticFunction(lua, "TimeHelper", "GetDateString", TimeHelper::GetDateString);
		ClassProxyHelper::PushStaticFunction(lua, "TimeHelper", "GetSecTimeStamp", TimeHelper::GetSecTimeStamp);
		ClassProxyHelper::PushStaticFunction(lua, "TimeHelper", "GetMilTimestamp", TimeHelper::GetMilTimestamp);
		ClassProxyHelper::PushStaticFunction(lua, "TimeHelper", "GetMicTimeStamp", TimeHelper::GetMicTimeStamp);
		ClassProxyHelper::PushStaticFunction(lua, "TimeHelper", "GetYearMonthDayString", TimeHelper::GetYearMonthDayString);
		
		ClassProxyHelper::PushStaticExtensionFunction(lua, "SoEasy", "Call", SystemExtension::Call);
		ClassProxyHelper::PushStaticExtensionFunction(lua, "SoEasy", "Start", SystemExtension::Start);
		ClassProxyHelper::PushStaticExtensionFunction(lua, "SoEasy", "Sleep", SystemExtension::Sleep);
		ClassProxyHelper::PushStaticExtensionFunction(lua, "SoEasy", "WaitFor", SystemExtension::WaitFor);
		ClassProxyHelper::PushStaticExtensionFunction(lua, "SoEasy", "CallWait", SystemExtension::CallWait);
		ClassProxyHelper::PushStaticExtensionFunction(lua, "SoEasy", "WaitNetFrame", SystemExtension::WaitNetFrame);

		ClassProxyHelper::PushStaticExtensionFunction(lua, "SoEasy", "AddTimer", SystemExtension::AddTimer);
		ClassProxyHelper::PushStaticExtensionFunction(lua, "SoEasy", "RemoveTimer", SystemExtension::RemoveTimer);

		ClassProxyHelper::PushStaticExtensionFunction(lua, "SoEasy", "GetManager", SystemExtension::GetManager);
		ClassProxyHelper::PushStaticExtensionFunction(lua, "SoEasy", "CreateByTable", LuaProtocExtension::CreateByTable);
	}

	XCode ScriptManager::OnHotfix(shared_ptr<TcpClientSession>, long long operId)
	{
		if (this->LoadAllModule())
		{
			return XCode::Successful;
		}
		return XCode::Failure;
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
		for (std::string & path : nAllLuaFile)
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

	void ScriptManager::RegisterExtension(lua_State * lua)
	{
		ClassProxyHelper::PushStaticExtensionFunction(lua, "SoEasy", "Log", LuaAPIExtension::DebugLog);
		ClassProxyHelper::PushStaticExtensionFunction(lua, "SoEasy", "Info", LuaAPIExtension::DebugInfo);
		ClassProxyHelper::PushStaticExtensionFunction(lua, "SoEasy", "Error", LuaAPIExtension::DebugError);
		ClassProxyHelper::PushStaticExtensionFunction(lua, "SoEasy", "Warning", LuaAPIExtension::DebugWarning);


		lua_newtable(lua);
		lua_pushtablefunction(lua, "Serialization", LuaProtocExtension::Serialization);
		lua_setglobal(lua, "ProtocUtil");

		lua_getglobal(lua, "coroutine");
		lua_pushtablefunction(lua, "sleep", CoroutineExtension::Sleep);
		lua_pushtablefunction(lua, "start", CoroutineExtension::Start);


	}
}