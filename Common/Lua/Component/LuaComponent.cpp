#include "LuaComponent.h"

#include "Entity/Actor/App.h"
#include "Util/Tools/Guid.h"
#include "Util/File/FileHelper.h"
#include "Util/Tools/String.h"
#include "Util/Tools/TimeHelper.h"
#include "Util/File/DirectoryHelper.h"
#include "Async/Lua/WaitLuaTaskSource.h"
#include "Rpc//Lua/LuaServiceTaskSource.h"
#include "Async/Lua/LuaCoroutine.h"
#include "Util/Crypt/LuaMd5.h"
#include "Core/System/System.h"
#include "Lua/Module/LuaModule.h"
#include "Entity/Lua/LuaActor.h"
#include "Lua/Engine/ClassProxyHelper.h"
#include "Lua/Engine/ModuleClass.h"
#include "Util/File/Lua/LuaFile.h"
#include "Yyjson/Lua/ljson.h"
#include "Proto/Message/IProto.h"
#include "Auth/Lua/Auth.h"
#include "Core/Excel/excel.h"
#include "Lua/Engine/Table.h"
#include "Net/Lua/LuaSocket.h"
#include "Net/Network/Tcp/Socket.h"

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
		const ServerConfig * config = ServerConfig::Inst();
		LOG_CHECK_RET_FALSE(config->Get("lua", jsonObject));
		LOG_CHECK_RET_FALSE(this->mLuaConfig->Init(*jsonObject));

		this->mLuaEnv = luaL_newstate();
		luaL_openlibs(mLuaEnv);
		this->RegisterLuaClass();
		return true;
	}

	void LuaComponent::RegisterLuaClass()
	{
		Lua::ClassProxyHelper os(this->mLuaEnv, "os");

		os.PushMember("dir", os::System::WorkPath());
		os.PushStaticFunction("setenv", os::System::LuaSetEnv);
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
		luaRegister1.PushStaticExtensionFunction("SetResult", WaitLuaTaskSource::SetResult);


		Lua::ClassProxyHelper luaRegister2(this->mLuaEnv, "LuaServiceTaskSource");
		luaRegister2.BeginRegister<LuaServiceTaskSource>();
		luaRegister2.PushExtensionFunction("SetRpc", LuaServiceTaskSource::SetRpc);
		luaRegister2.PushExtensionFunction("SetHttp", LuaServiceTaskSource::SetHttp);

		Lua::ClassProxyHelper luaRegister4(this->mLuaEnv, "coroutine");
		luaRegister4.PushExtensionFunction("start", Lua::Coroutine::Start);
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

		Lua::ClassProxyHelper classProxyHelper4(this->mLuaEnv, "Socket");
		classProxyHelper4.BeginRegister<tcp::Socket>();
		classProxyHelper4.PushExtensionFunction("Send", Lua::Sock::Send);
		classProxyHelper4.PushExtensionFunction("Read", Lua::Sock::Read);
		classProxyHelper4.PushExtensionFunction("Query", Lua::Sock::Query);
		classProxyHelper4.PushExtensionFunction("Close", Lua::Sock::Close);
		classProxyHelper4.PushExtensionFunction("Connect", Lua::Sock::Connect);
	}

	bool LuaComponent::LateAwake()
	{
		Lua::ModuleClass moduleRegistry(this->mLuaEnv);

		moduleRegistry.Start();
		moduleRegistry.AddFunction("Find", Lua::LuaFile::Find);
		moduleRegistry.AddFunction("GetMd5", Lua::LuaFile::GetMd5);
		moduleRegistry.AddFunction("IsExist", Lua::LuaFile::IsExist);
		moduleRegistry.AddFunction("GetFiles", Lua::LuaFile::GetFiles);
		moduleRegistry.AddFunction("GetFileName", Lua::LuaFile::GetFileName);
		moduleRegistry.AddFunction("GetLastWriteTime", Lua::LuaFile::GetLastWriteTime).End("util.fs");

		moduleRegistry.Start();
		moduleRegistry.AddFunction("Stop", LuaActor::Stop);
		moduleRegistry.AddFunction("Random", LuaActor::Random);
		moduleRegistry.AddFunction("GetPath", LuaActor::GetPath);
		moduleRegistry.AddFunction("NewGuid", LuaActor::NewGuid);
		moduleRegistry.AddFunction("NewUuid", LuaActor::NewUuid);
		moduleRegistry.AddFunction("GetListen", LuaActor::GetListen);
		moduleRegistry.AddFunction("GetConfig", LuaActor::GetConfig);
		moduleRegistry.AddFunction("GetServers", LuaActor::GetServers);
		moduleRegistry.AddFunction("MakeServer", LuaActor::MakeServer);
		moduleRegistry.AddFunction("HasComponent", LuaActor::HasComponent);
		moduleRegistry.AddFunction("AddComponent", LuaActor::AddComponent).End("core.app");

		moduleRegistry.Start();
		moduleRegistry.AddFunction("read", lua::yyjson::read_file);
		moduleRegistry.AddFunction("encode", lua::yyjson::encode);
		moduleRegistry.AddFunction("pretty", lua::yyjson::pretty);
		moduleRegistry.AddFunction("decode", lua::yyjson::decode).End("util.json");

		moduleRegistry.Start();
		moduleRegistry.AddFunction("ToString", Lua::Md5::ToString).End("util.md5");

		moduleRegistry.Start();
		moduleRegistry.AddFunction("Decode", Lua::base64::Decode);
		moduleRegistry.AddFunction("Encode", Lua::base64::Encode).End("util.base64");

		moduleRegistry.Start();
		moduleRegistry.AddFunction("Create", lua::ljwt::Create);
		moduleRegistry.AddFunction("Verify", lua::ljwt::Verify).End("auth.jwt");

		moduleRegistry.Start();
		moduleRegistry.AddFunction("Make", Lua::LuaDir::Make);
		moduleRegistry.AddFunction("IsExist", Lua::LuaDir::IsExist).End("util.dir");


		std::vector<ILuaRegister*> components;
		this->mApp->GetComponents(components);
		for (ILuaRegister* component: components)
		{
			moduleRegistry.Start();
			component->OnLuaRegister(moduleRegistry);
		}
		return this->LoadAllFile();
	}

	Lua::LuaModule * LuaComponent::LoadModule(const std::string& name)
	{
		LuaModule * luaModule = nullptr;
		if(this->mLuaModules.Get(name, luaModule))
		{
			return luaModule;
		}
		if(!this->mModulePaths.Has(name))
		{
			return nullptr;
		}
		lua_getglobal(this->mLuaEnv, "require");
		lua_pushstring(this->mLuaEnv, name.c_str());
		if(lua_pcall(this->mLuaEnv, 1, 1, 0) != LUA_OK)
		{
			LOG_ERROR("{}", lua_tostring(this->mLuaEnv, -1));
			return nullptr;
		}

		luaL_checktype(this->mLuaEnv, -1, LUA_TTABLE);
		int ref = luaL_ref(this->mLuaEnv, LUA_REGISTRYINDEX);
		luaModule = new LuaModule(this->mLuaEnv, name, ref);
		{
			this->mLuaModules.Add(name, luaModule);
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
		std::string work = os::System::WorkPath() + '/';
		std::unordered_set<std::string> directors;
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
					ModuleInfo * moduleInfo = new ModuleInfo();
					{
						moduleInfo->Name = moduleName;
						moduleInfo->FullPath = filePath;
						moduleInfo->LastWriteTime = time;
						moduleInfo->LocalPath = filePath;
						help::Str::ReplaceString(moduleInfo->LocalPath, work, "");
					}
					if(!this->mModulePaths.Add(moduleName, moduleInfo))
					{
						LOG_ERROR("module name : {}  {}", moduleName, filePath);
						return false;
					}
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
		auto iter = this->mModulePaths.Begin();
		for (; iter != this->mModulePaths.End(); iter++)
		{
			this->CheckModuleHotfix(iter->first);
		}
		return true;
	}

	void LuaComponent::CheckModuleHotfix(const std::string& module)
	{
		ModuleInfo * moduleInfo = nullptr;
		if (!this->mModulePaths.Get(module, moduleInfo))
		{
			return;
		}
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
		LuaModule * luaModule = nullptr;
		if (this->mLuaModules.Get(module, luaModule))
		{
			luaModule->OnModuleHotfix();
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
			return 0;
		}
		return lua_tonumber(this->mLuaEnv, -1);
	}

	void LuaComponent::OnRecord(json::w::Document& document)
	{
		document.Add("lua", (int)this->GetMemorySize());
	}
}