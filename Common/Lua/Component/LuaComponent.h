#pragma once
#include<set>
#include<memory>
#include<unordered_map>
#include"Lua/Module/LuaModule.h"
#include"Lua/Config/LuaConfig.h"
#include"Entity/Component/Component.h"
struct lua_State;
namespace acs
{
	class ModuleInfo
	{
	public:
		std::string Name;
		std::string FullPath; //完整路径
		std::string LocalPath; //相对路径
		long long LastWriteTime;
	};

    class LuaComponent final : public Component,
			public IStart, public IComplete, public IHotfix,
			public IServerRecord, public IDestroy
	{
	 public:
		LuaComponent();
		~LuaComponent() final = default;
    public:
		double GetMemorySize();
		double CollectGarbage();
		Lua::LuaModule * LoadModule(const std::string & name);
	protected:
		bool Awake() final;
		void Start() final;
		bool LateAwake() final;
		void OnDestroy() final;
		void Complete() final;
		bool OnHotFix() final;
		void OnRecord(json::w::Document &document) final;
    private:
		void LoadAllLib();
		bool LoadAllFile();
		void RegisterLuaClass();
		void AddRequire(const std::string & direct);
		void CheckModuleHotfix(const std::string & module);
	private:
        lua_State* mLuaEnv;
		std::string mModulePath;
		std::string mComponentPath;
		Lua::LuaModule * mMainModule;
		std::vector<std::string> mDoFiles;
		std::unique_ptr<LuaConfig> mLuaConfig;
		std::unordered_map<std::string, std::unique_ptr<ModuleInfo>> mModulePaths;
		std::unordered_map<std::string, std::unique_ptr<Lua::LuaModule>> mLuaModules;
	};
}