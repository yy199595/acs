#pragma once
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
		std::string FullName;
		std::string FullPath; //完整路径
		std::string LocalPath; //相对路径
		long long LastWriteTime;
	};

	class LuaComponent final : public Component,
							   public IStart, public IComplete, public IRefresh,
							   public IServerRecord, public IAppStop, public ISecondUpdate, public IDestroy
	{
	public:
		LuaComponent();
		~LuaComponent() final = default;
	public:
		double GetMemorySize();
		Lua::LuaModule * LoadModule(const std::string & name);
	protected:
		bool Awake() final;
		void OnStart() final;
		bool LateAwake() final;
		void OnAppStop() final;
		void OnComplete() final;
		bool OnRefresh() final;
		void OnDestroy() final;
		void OnSecondUpdate(int tick) noexcept final;
		void OnRecord(json::w::Document &document) final;
	private:
		void LoadAllLib();
		bool LoadAllFile();
		void RegisterLuaClass();
		void CollectCollectgarbage() const;
		void AddRequire(const std::string & direct);
		void RefreshLuaModule(const std::string & module);
	private:
		lua_State* mLuaEnv;
		lua::Config mConfig;
		std::string mModulePath;
		std::string mComponentPath;
		std::vector<std::string> mDoFiles;
		std::unordered_map<std::string, std::unique_ptr<ModuleInfo>> mModulePaths;
		std::unordered_map<std::string, std::unique_ptr<Lua::LuaModule>> mLuaModules;
	};
}