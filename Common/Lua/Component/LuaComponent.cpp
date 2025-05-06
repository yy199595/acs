#include "LuaComponent.h"

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
#include "Lua/Socket/LuaTcpSocket.h"

#ifdef __ENABLE_OPEN_SSL__

#include "Util/Ssl/rsa.h"
#include "Util/Ssl/LuaRsa.h"
#include "Yyjson/Lua/ljson.h"

#endif



using namespace Lua;
namespace acs
{
	LuaComponent::LuaComponent()
	{
		this->mLuaEnv = nullptr;
		REGISTER_JSON_CLASS_FIELD(lua::Config, main);
		REGISTER_JSON_CLASS_FIELD(lua::Config, require);
		REGISTER_JSON_CLASS_FIELD(lua::Config, modules);
	}

	bool LuaComponent::Awake()
	{
		ServerConfig* config = ServerConfig::Inst();
		LOG_CHECK_RET_FALSE(config->Get("lua", this->mConfig));
		this->mLuaEnv = luaL_newstate();
		{
			luaL_openlibs(this->mLuaEnv);
			this->RegisterLuaClass();
			lua_pushglobalfunction(this->mLuaEnv, "print", lua::lfmt::lprint);
		}
		return true;
	}

	void LuaComponent::LoadAllLib()
	{
		Lua::CCModule moduleRegistry(this->mLuaEnv);
		moduleRegistry.Open("util.fs", lua::lib::luaopen_lfs);
		moduleRegistry.Open("util.md5", lua::lib::luaopen_lmd5);
		moduleRegistry.Open("util.zip", lua::lib::luaopen_lzip);
#ifdef __ENABLE_OPEN_SSL__
		moduleRegistry.Open("util.rsa", lua::lib::luaopen_lrsa);
		moduleRegistry.Open("util.aes", lua::lib::luaopen_lraes);
#endif
		moduleRegistry.Open("util.jwt", lua::lib::luaopen_ljwt);
		moduleRegistry.Open("util.sha1", lua::lib::luaopen_lsha1);

		moduleRegistry.Open("util.fmt", lua::lib::luaopen_lfmt);
		moduleRegistry.Open("util.json", lua::lib::luaopen_ljson);
		moduleRegistry.Open("util.bson", lua::lib::luaopen_lbson);
		moduleRegistry.Open("util.guid", lua::lib::luaopen_lguid);
		moduleRegistry.Open("util.pb", lua::lib::luaopen_lproto);
		moduleRegistry.Open("util.base64", lua::lib::luaopen_lbase64);

		moduleRegistry.Open("core.app", lua::lib::luaopen_lapp);
		moduleRegistry.Open("core.node", lua::lib::luaopen_lnode);
		moduleRegistry.Open("core.actor", lua::lib::luaopen_lactor);

		moduleRegistry.Open("net.tcp", lua::lib::luaopen_ltcp);
		moduleRegistry.Open("util.lxlsx", lua::lib::luaopen_lexcel);

		LuaCCModuleRegister::Trigger(moduleRegistry);
	}

	void LuaComponent::RegisterLuaClass()
	{
		Lua::ClassProxyHelper os(this->mLuaEnv, "os");

		os.PushMember("dir", os::System::WorkPath());
		os.PushStaticFunction("setenv", os::System::LuaSetEnv);
		os.PushExtensionFunction("run", LuaCore::Run);
		os.PushExtensionFunction("get_system_info", LuaCore::GetSystemInfo);
#ifdef __OS_MAC__
		os.PushMember("platform", std::string("mac"));
#elif __OS_LINUX__
		os.PushMember("platform", std::string("linux"));
#elif __OS_WIN__
		os.PushMember("platform", std::string("win"));
		os.PushStaticFunction("SetConsoleTitle", SetConsoleTitle);
#endif

#ifdef __DEBUG__
		os.PushMember("debug", true);
#else
		os.PushMember("debug", false);
#endif
		os.BeginNewTable("time");
		os.PushStaticFunction("ms", help::Time::NowMil);
		os.PushStaticFunction("now", help::Time::NowSec);
		os.PushStaticFunction("date", help::Time::GetDateString);
		os.PushStaticFunction("from", help::Time::GetTimeByString);

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


		Lua::ClassProxyHelper classProxyHelper10(this->mLuaEnv, "JsonValue");
		classProxyHelper10.BeginRegister<lua::JsonValue>();
		classProxyHelper10.PushExtensionFunction("encode", lua::ljson::encode);
		classProxyHelper10.PushExtensionFunction("add_member", lua::ljson::add_member);
		classProxyHelper10.PushExtensionFunction("add_object", lua::ljson::add_object);
		classProxyHelper10.PushExtensionFunction("add_array", lua::ljson::add_array);

	}

	bool LuaComponent::LateAwake()
	{
		this->LoadAllLib();
		return this->LoadAllFile();
	}

	Lua::LuaModule* LuaComponent::LoadModule(const std::string& name)
	{
		if(name.empty())
		{
			return nullptr;
		}
		auto iter = this->mLuaModules.find(name);
		if (iter != this->mLuaModules.end())
		{
			return iter->second.get();
		}
		auto iter1 = this->mModulePaths.find(name);
		if (iter1 == this->mModulePaths.end())
		{
			return nullptr;
		}
		lua_settop(this->mLuaEnv, 0);
		const std::string & path = iter1->second->FullPath;
		if(luaL_dofile(this->mLuaEnv, path.c_str()) != LUA_OK)
		{
			LOG_ERROR("{}", lua_tostring(this->mLuaEnv, -1));
			lua_pop(this->mLuaEnv, 1);
			return nullptr;
		}
		LuaModule* luaModule = nullptr;
		if(!lua_istable(this->mLuaEnv, -1))
		{
			LOG_ERROR("lua module [{}] return not table", name);
			return nullptr;
		}
		int ref = luaL_ref(this->mLuaEnv, LUA_REGISTRYINDEX);
		std::unique_ptr<LuaModule> newModule = std::make_unique<LuaModule>(this->mLuaEnv, name, ref);
		{
			if(newModule->GetFunction("OnAwake"))
			{
				if(lua_pcall(this->mLuaEnv, 1, 0, 0) != LUA_OK)
				{
					LOG_ERROR("[{}] => {}", name, lua_tostring(this->mLuaEnv, -1));
					lua_pop(this->mLuaEnv, 1);
					return nullptr;
				}
			}
			luaModule = newModule.get();
			LOG_DEBUG("load lua module => [{}] ok", name)
			this->mLuaModules.emplace(name, std::move(newModule));
		}
		return luaModule;
	}

	void LuaComponent::OnStart()
	{
		for(auto iter = this->mLuaModules.begin(); iter != this->mLuaModules.end(); iter++)
		{
			iter->second->Await("OnStart");
		}
	}

	void LuaComponent::OnComplete()
	{
		for(auto iter = this->mLuaModules.begin(); iter != this->mLuaModules.end(); iter++)
		{
			iter->second->Await("OnComplete");
		}
	}

	bool LuaComponent::LoadAllFile()
	{
		std::vector<std::string> loadModules;
		std::string work = os::System::WorkPath() + '/';
		for (const std::string& path: this->mConfig.require)
		{
			std::vector<std::string> files;
			help::dir::GetFilePaths(path, "*.lua", files);
			for (const std::string& filePath: files)
			{
				std::string moduleName;
				long long time = help::fs::GetLastWriteTime(filePath);
				if (help::fs::GetFileName(filePath, moduleName))
				{
					auto iter = this->mModulePaths.find(moduleName);
					if (iter != this->mModulePaths.end())
					{
						LOG_ERROR("module name : {}  {}", moduleName, filePath);
						return false;
					}
					std::string fullName = filePath.substr(path.size() + 1);
					{
						help::Str::ReplaceString(fullName, ".lua", "");
						help::Str::ReplaceString(fullName, "/", ".");
					}
					std::unique_ptr<ModuleInfo> moduleInfo = std::make_unique<ModuleInfo>();
					{
						moduleInfo->Name = moduleName;
						moduleInfo->FullPath = filePath;
						moduleInfo->LastWriteTime = time;
						moduleInfo->LocalPath = filePath;
						moduleInfo->FullName = fullName;
						help::Str::ReplaceString(moduleInfo->LocalPath, work, "");
					}
					this->mModulePaths.emplace(moduleName, std::move(moduleInfo));
				}
			}
		}
		for (const std::string& director: this->mConfig.require)
		{
			this->AddRequire(director);
		}
		this->LoadModule(this->mConfig.main);
		for(const std::string & module : this->mConfig.modules)
		{
			if(this->LoadModule(module) == nullptr)
			{
				LOG_ERROR("load module => [{}] fail", module);
				return false;
			}
		}
		return true;
	}

	bool LuaComponent::OnRefresh()
	{
		for (auto iter = this->mModulePaths.begin(); iter != this->mModulePaths.end(); iter++)
		{
			this->RefreshLuaModule(iter->first);
		}
		for(auto iter1 = this->mLuaModules.begin(); iter1 != this->mLuaModules.end(); iter1++)
		{
			iter1->second->Await("OnRefresh");
		}
		return true;
	}

	void LuaComponent::OnSecondUpdate(int tick) noexcept
	{
		for(auto iter = this->mLuaModules.begin(); iter != this->mLuaModules.end(); iter++)
		{
			iter->second->Call("OnUpdate", tick);
		}

	}

	void LuaComponent::RefreshLuaModule(const std::string& module)
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

	void LuaComponent::OnAppStop()
	{
		for(auto iter = this->mLuaModules.begin(); iter != this->mLuaModules.end(); iter++)
		{
			LOG_INFO("close {}", iter->first);
			iter->second->Await("OnDestroy");
		}
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
			std::string fullPath = fmt::format("{}/?.lua;{}", path, std::string(str, size));
			lua_pushlstring(this->mLuaEnv, fullPath.c_str(), fullPath.size());
			lua_setfield(this->mLuaEnv, -3, "path");
		}
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

	void LuaComponent::CollectCollectgarbage() const
	{
		lua_settop(this->mLuaEnv, 0);
		lua_getglobal(this->mLuaEnv, "collectgarbage");

		lua_pushstring(this->mLuaEnv, "collect");
		if (lua_pcall(this->mLuaEnv, 1, 1, 0) != LUA_OK)
		{
			LOG_ERROR(lua_tostring(this->mLuaEnv, -1));
			lua_pop(this->mLuaEnv, 1);
		}
	}


	void LuaComponent::OnRecord(json::w::Document& document)
	{
		std::unique_ptr<json::w::Value> jsonObject = document.AddObject("lua");
		{
			double memory = this->GetMemorySize();
			jsonObject->Add("module", this->mLuaModules.size());
			jsonObject->Add("memory", fmt::format("{:.2f}KB", memory));
		}
	}
}