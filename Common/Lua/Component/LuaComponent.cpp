﻿#include "LuaComponent.h"

#include "Entity/Actor/App.h"
#include "Util/File/FileHelper.h"
#include "Util/Tools/String.h"
#include "Util/Tools/TimeHelper.h"
#include "Util/File/DirectoryHelper.h"
#include "Async/Lua/WaitLuaTaskSource.h"
#include "Rpc//Lua/LuaServiceTaskSource.h"
#include "Async/Lua/LuaCoroutine.h"
#include "Core/System/System.h"
#include "Lua/Module/LuaModule.h"
#include "Lua/Engine/ClassProxyHelper.h"
#include "Lua/Engine/ModuleClass.h"
#include "Proto/Message/IProto.h"
#include "Core/Excel/excel.h"
#include "Net/Lua/LuaSocket.h"
#include "Net/Network/Tcp/Socket.h"
#include "Core/Lua/LuaOs.h"
#include "Lua/Lib/Lib.h"

#ifdef __ENABLE_OPEN_SSL__

#include "Util/Ssl/rsa.h"
#include "Util/Ssl/LuaRsa.h"
#include "Lua/Socket/LuaTcpSocket.h"

#endif
using namespace Lua;
namespace acs
{
	LuaComponent::LuaComponent()
	{
		this->mLuaEnv = nullptr;
		this->mMainModule = nullptr;
	}

	bool LuaComponent::Awake()
	{
		std::unique_ptr<json::r::Value> jsonObject;
		this->mLuaConfig = std::make_unique<LuaConfig>();
		const ServerConfig* config = ServerConfig::Inst();
		LOG_CHECK_RET_FALSE(config->Get("lua", jsonObject));
		LOG_CHECK_RET_FALSE(this->mLuaConfig->Init(*jsonObject));

		this->mLuaEnv = luaL_newstate();
		this->LoadAllLib();
		this->RegisterLuaClass();
		return true;
	}

	void LuaComponent::LoadAllLib()
	{
		luaL_openlibs(this->mLuaEnv);
		this->AddCCModule("util.fs", lua::lib::luaopen_lfs);
		this->AddCCModule("util.md5", lua::lib::luaopen_lmd5);
		this->AddCCModule("util.rsa", lua::lib::luaopen_lrsa);
		this->AddCCModule("util.aes", lua::lib::luaopen_lraes);

		this->AddCCModule("util.oss", lua::lib::luaopen_loss);
		this->AddCCModule("util.fmt", lua::lib::luaopen_lfmt);
		this->AddCCModule("util.jwt", lua::lib::luaopen_ljwt);
		this->AddCCModule("util.json", lua::lib::luaopen_ljson);
		this->AddCCModule("util.bson", lua::lib::luaopen_lbson);
		this->AddCCModule("util.proto", lua::lib::luaopen_lproto);
		this->AddCCModule("util.base64", lua::lib::luaopen_lbase64);

		this->AddCCModule("db.redis", lua::lib::luaopen_lredisdb);
		this->AddCCModule("db.mongo", lua::lib::luaopen_lmonogodb);
		this->AddCCModule("db.sqlite", lua::lib::luaopen_lsqlitedb);

		this->AddCCModule("core.app", lua::lib::luaopen_lapp);
		this->AddCCModule("core.log", lua::lib::luaopen_llog);
		this->AddCCModule("core.timer", lua::lib::luaopen_ltimer);

		this->AddCCModule("net.tcp", lua::lib::luaopen_ltcp);
		this->AddCCModule("net.http", lua::lib::luaopen_lhttp);
	}

	void LuaComponent::AddCCModule(const char* module, lua_CFunction func)
	{
		Lua::ModuleClass moduleRegistry(this->mLuaEnv);
		moduleRegistry.Register(module, func);
	}

	void LuaComponent::RegisterLuaClass()
	{
		Lua::ClassProxyHelper os(this->mLuaEnv, "os");

		os.PushMember("dir", os::System::WorkPath());
		os.PushStaticFunction("setenv", os::System::LuaSetEnv);
		os.PushExtensionFunction("get_system_info", LuaCore::GetSystemInfo);
#ifdef __OS_MAC__
		os.PushMember("platform", std::string("mac"));
#elif __OS_LINUX__
		os.PushMember("platform", std::string("linux"));
#elif __OS_WIN__
		os.PushMember("platform", std::string("win"));
#endif

#ifdef __DEBUG__
		os.PushMember("debug", true);
#else
		os.PushMember("debug", false);
#endif
		os.BeginNewTable("time");
		os.PushStaticFunction("ms", help::Time::NowMil);
		os.PushStaticFunction("now", help::Time::NowSec);

		Lua::ClassProxyHelper luaHttpRegister(this->mLuaEnv, "Head");
		luaHttpRegister.BeginRegister<tcp::IHeader>();
		luaHttpRegister.PushExtensionFunction("Get", tcp::IHeader::LuaGet);
		luaHttpRegister.PushExtensionFunction("ToString", tcp::IHeader::LuaToString);

		Lua::ClassProxyHelper luaRegister1(this->mLuaEnv, "WaitLuaTaskSource");
		luaRegister1.BeginRegister<WaitLuaTaskSource>();
		luaRegister1.PushCtor<WaitLuaTaskSource>();
		luaRegister1.PushExtensionFunction("SetResult", WaitLuaTaskSource::SetResult);


		Lua::ClassProxyHelper luaRegister2(this->mLuaEnv, "LuaServiceTaskSource");
		luaRegister2.BeginRegister<LuaServiceTaskSource>();
		luaRegister2.PushExtensionFunction("SetRpc", LuaServiceTaskSource::SetRpc);
		luaRegister2.PushExtensionFunction("SetHttp", LuaServiceTaskSource::SetHttp);

		Lua::ClassProxyHelper luaRegister4(this->mLuaEnv, "coroutine");
		luaRegister4.PushExtensionFunction("sleep", Lua::Coroutine::Sleep);

		Lua::ClassProxyHelper classProxyHelper1(this->mLuaEnv, "Cell");
		classProxyHelper1.BeginRegister<lxlsx::Cell>();
		classProxyHelper1.PushMemberField<lxlsx::Cell>("type", &lxlsx::Cell::type);
		classProxyHelper1.PushMemberField<lxlsx::Cell>("value", &lxlsx::Cell::value);
		classProxyHelper1.PushMemberField<lxlsx::Cell>("fmt_id", &lxlsx::Cell::fmt_id);
		classProxyHelper1.PushMemberField<lxlsx::Cell>("fmt_code", &lxlsx::Cell::fmt_code);

		Lua::ClassProxyHelper classProxyHelper2(this->mLuaEnv, "Sheet");
		classProxyHelper2.BeginRegister<lxlsx::Sheet>();
		classProxyHelper2.PushMemberField<lxlsx::Sheet>("name", &lxlsx::Sheet::name);
		classProxyHelper2.PushMemberField<lxlsx::Sheet>("last_row", &lxlsx::Sheet::last_row);
		classProxyHelper2.PushMemberField<lxlsx::Sheet>("last_col", &lxlsx::Sheet::last_col);
		classProxyHelper2.PushMemberField<lxlsx::Sheet>("first_row", &lxlsx::Sheet::first_row);
		classProxyHelper2.PushMemberField<lxlsx::Sheet>("first_col", &lxlsx::Sheet::first_col);
		classProxyHelper2.PushMemberFunction<lxlsx::Sheet>("get_cell", &lxlsx::Sheet::get_cell);


		Lua::ClassProxyHelper classProxyHelper3(this->mLuaEnv, "ExcelFile");
		classProxyHelper3.BeginRegister<lxlsx::ExcelFile>();
		classProxyHelper3.PushCtor<lxlsx::ExcelFile>();
		classProxyHelper3.PushMemberFunction("Open", &lxlsx::ExcelFile::Open);
		classProxyHelper3.PushMemberFunction("GetSheet", &lxlsx::ExcelFile::GetSheet);
		classProxyHelper3.PushMemberFunction("GetSheets", &lxlsx::ExcelFile::GetSheets);

		Lua::ClassProxyHelper classProxyHelper4(this->mLuaEnv, "TcpSocket");
		classProxyHelper4.BeginRegister<lua::TcpClient>();
		classProxyHelper4.PushExtensionFunction("send", lua::TcpSock::Send);
		classProxyHelper4.PushExtensionFunction("close", lua::TcpSock::Close);
		classProxyHelper4.PushExtensionFunction("receive", lua::TcpSock::Read);
		classProxyHelper4.PushExtensionFunction("set_timeout", lua::TcpSock::SetTimeout);

		Lua::ClassProxyHelper classProxyHelper44(this->mLuaEnv, "TcpAcceptor");
		classProxyHelper44.BeginRegister<Asio::Acceptor>();
		classProxyHelper44.PushExtensionFunction("accept", lua::TcpSock::Accept);

#ifdef __ENABLE_OPEN_SSL__
		Lua::ClassProxyHelper classProxyHelper7(this->mLuaEnv, "RSAEncryptor");
		classProxyHelper7.BeginRegister<ssl::RSAEncryptor>();
		classProxyHelper7.PushExtensionFunction("encode", lua::rsa::Encode);
		classProxyHelper7.PushExtensionFunction("decode", lua::rsa::Decode);
#endif

		Lua::ClassProxyHelper classProxyHelper8(this->mLuaEnv, "string");
		classProxyHelper8.PushStaticFunction("range", help::Str::RandomString);

		Lua::ClassProxyHelper classProxyHelper9(this->mLuaEnv, "table");
		classProxyHelper9.PushExtensionFunction("serialize", lua::lfmt::serialize);
		classProxyHelper9.PushExtensionFunction("deserialize", lua::lfmt::deserialize);
	}

	bool LuaComponent::LateAwake()
	{
		return this->LoadAllFile();
	}

	Lua::LuaModule* LuaComponent::LoadModule(const std::string& name)
	{
		auto iter = this->mLuaModules.find(name);
		if (iter != this->mLuaModules.end())
		{
			return iter->second.get();
		}

		if (this->mModulePaths.find(name) == this->mModulePaths.end())
		{
			return nullptr;
		}
		lua_settop(this->mLuaEnv, 0);
		lua_getglobal(this->mLuaEnv, "require");
		assert(lua_isfunction(this->mLuaEnv, -1));
		lua_pushstring(this->mLuaEnv, name.c_str());
		if (lua_pcall(this->mLuaEnv, 1, 1, 0) != LUA_OK)
		{
			LOG_ERROR("{}", lua_tostring(this->mLuaEnv, -1));
			lua_pop(this->mLuaEnv, 1);
			return nullptr;
		}
		LuaModule* luaModule = nullptr;
		luaL_checktype(this->mLuaEnv, -1, LUA_TTABLE);
		int ref = luaL_ref(this->mLuaEnv, LUA_REGISTRYINDEX);
		std::unique_ptr<LuaModule> newModule = std::make_unique<LuaModule>(this->mLuaEnv, name, ref);
		{
			luaModule = newModule.get();
			this->mLuaModules.emplace(name, std::move(newModule));
		}
		return luaModule;
	}

	void LuaComponent::Start()
	{
		IF_NOT_NULL_CALL(this->mMainModule, Await, "OnStart");
	}

	void LuaComponent::Complete()
	{
		IF_NOT_NULL_CALL(this->mMainModule, Await, "OnComplete");
	}

	bool LuaComponent::LoadAllFile()
	{
		std::vector<std::string> loadModules;
		std::unordered_set<std::string> directors;
		std::string work = os::System::WorkPath() + '/';
		for (const std::string& path: this->mLuaConfig->Requires())
		{
			std::vector<std::string> files;
			help::dir::GetFilePaths(path, "*.lua", files);
			for (const std::string& filePath: files)
			{
				std::string director;
				std::string moduleName;
				if (help::dir::GetDirByPath(filePath, director))
				{
					directors.insert(director);
				}
				long long time = help::fs::GetLastWriteTime(filePath);
				if (help::fs::GetFileName(filePath, moduleName))
				{
					auto iter = this->mModulePaths.find(moduleName);
					if (iter != this->mModulePaths.end())
					{
						LOG_ERROR("module name : {}  {}", moduleName, filePath);
						return false;
					}
					std::unique_ptr<ModuleInfo> moduleInfo = std::make_unique<ModuleInfo>();
					{
						moduleInfo->Name = moduleName;
						moduleInfo->FullPath = filePath;
						moduleInfo->LastWriteTime = time;
						moduleInfo->LocalPath = filePath;
						help::Str::ReplaceString(moduleInfo->LocalPath, work, "");
					}
					this->mModulePaths.emplace(moduleName, std::move(moduleInfo));
				}
			}
		}
		for (const std::string& director: directors)
		{
			this->AddRequire(director);
		}

		if (this->mLuaConfig->Main().empty())
		{
			return true;
		}
		std::string main = this->mLuaConfig->Main();
		this->mMainModule = this->LoadModule(main);
		if (this->mMainModule != nullptr)
		{
			int code = this->mMainModule->Call("Awake");
			if (code == XCode::CallLuaFunctionFail)
			{
				return false;
			}
		}
		return true;
	}

	bool LuaComponent::OnHotFix()
	{
		auto iter = this->mModulePaths.begin();
		for (; iter != this->mModulePaths.end(); iter++)
		{
			this->CheckModuleHotfix(iter->first);
		}
		this->CollectGarbage();
		return true;
	}

	void LuaComponent::CheckModuleHotfix(const std::string& module)
	{
		auto iter = this->mModulePaths.find(module);
		if (iter == this->mModulePaths.end())
		{
			return;
		}
		ModuleInfo* moduleInfo = iter->second.get();
		const std::string& path = moduleInfo->FullPath;
		long long time = help::fs::GetLastWriteTime(path);
		if (moduleInfo->LastWriteTime == time)
		{
			return;
		}
		moduleInfo->LastWriteTime = time;
		if (luaL_dofile(this->mLuaEnv, path.c_str()) != LUA_OK)
		{
			LOG_ERROR(lua_tostring(this->mLuaEnv, -1));
			return;
		}
		auto iter1 = this->mLuaModules.find(module);
		if (iter1 != this->mLuaModules.end())
		{
			iter1->second->OnModuleHotfix();
		}
		LOG_INFO("load lua file [{}] ok", moduleInfo->LocalPath);
	}

	void LuaComponent::OnDestroy()
	{
		IF_NOT_NULL_CALL(this->mMainModule, Await, "OnStop");
	}

	void LuaComponent::AddRequire(const std::string& path)
	{
		if (!path.empty())
		{
			size_t size = 0;
			lua_getglobal(this->mLuaEnv, "package");
			lua_getfield(this->mLuaEnv, -1, "path");
			const char* str = lua_tolstring(this->mLuaEnv, -1, &size);
			//std::string fullPath = std::string(str, size) + ";" + path + "?.lua";
			std::string fullPath = fmt::format("{}?.lua;{}", path, std::string(str, size));
			lua_pushlstring(this->mLuaEnv, fullPath.c_str(), fullPath.size());
			lua_setfield(this->mLuaEnv, -3, "path");
		}
	}

	double LuaComponent::CollectGarbage()
	{
		lua_settop(this->mLuaEnv, 0);
		double start = this->GetMemorySize();
		lua_getglobal(this->mLuaEnv, "collectgarbage");

		lua_pushstring(this->mLuaEnv, "collect");
		if (lua_pcall(this->mLuaEnv, 1, 1, 0) != LUA_OK)
		{
			LOG_ERROR(lua_tostring(this->mLuaEnv, -1));
			lua_pop(this->mLuaEnv, 1);
			return 0;
		}
		return start - this->GetMemorySize();
	}

	double LuaComponent::GetMemorySize()
	{
		lua_settop(this->mLuaEnv, 0);
		lua_getglobal(this->mLuaEnv, "collectgarbage");

		lua_pushstring(this->mLuaEnv, "count");
		if (lua_pcall(this->mLuaEnv, 1, 1, 0) != LUA_OK)
		{
			LOG_ERROR(lua_tostring(this->mLuaEnv, -1));
			lua_pop(this->mLuaEnv, 1);
			return 0;
		}
		return lua_tonumber(this->mLuaEnv, -1);
	}

	void LuaComponent::OnRecord(json::w::Document& document)
	{
		std::unique_ptr<json::w::Value> jsonObject = document.AddObject("lua");
		{
			document.Add("lua", this->GetMemorySize());
			document.Add("free", this->CollectGarbage());
			document.Add("module", this->mModulePaths.size());
		}
	}
}