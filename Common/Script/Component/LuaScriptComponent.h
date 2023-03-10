#pragma once
#include<set>
#include<memory>
#include<unordered_map>
#include"Component/Component.h"
namespace Lua
{
    class LuaModule;
}
struct lua_State;
namespace Sentry
{
    class LuaScriptComponent : public Component, public IStart, 
							   public IComplete, public IHotfix, public ISecondUpdate, public IServerRecord
	{
	 public:
		LuaScriptComponent() = default;
		virtual ~LuaScriptComponent() = default;
    public:
		double GetMemorySize();
		double CollectGarbage();
		Lua::LuaModule * GetModule(const std::string & name);
		Lua::LuaModule * LoadModule(const std::string & name);
		struct lua_State* GetLuaEnv() { return this->mLuaEnv; }
        bool GetFunction(const std::string & tab, const std::string & func);
	 protected:
		bool Awake() final;
		bool Start() final;
		bool LateAwake() final;
		void OnDestroy() final;
		void OnLocalComplete() final;
		void OnClusterComplete() final;
        void OnSecondUpdate(const int tick) final;
        void OnHotFix() final;
		void OnRecord(Json::Writer &document) final;
    private:
		bool LoadAllFile();
		bool LoadAllFilePath(const std::string & dir);
        void AddRequire(const std::string & direct);
	 private:
        lua_State* mLuaEnv;
        std::string mModulePath;
        std::set<std::string> mDirectorys;
		std::unordered_map<std::string, int> mFuncs;
		std::unordered_map<std::string, std::string> mModulePaths;
		std::unordered_map<std::string, std::unique_ptr<Lua::LuaModule>> mModules;
	};
}