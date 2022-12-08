#pragma once

#include"Lua/WaitLuaTaskSource.h"
#include"Component/Component.h"
#include"Lua/Table.h"
#include"Lua/LocalTable.h"
#include"Module/LuaModule.h"
#include"Lua/ClassProxyHelper.h"
namespace Sentry
{
    class LuaScriptComponent : public Component, public IStart, 
		public IComplete, public IHotfix, public ISecondUpdate
	{
	 public:
		LuaScriptComponent() = default;
		virtual ~LuaScriptComponent() = default;
    public:
		Lua::LuaModule * GetModule(const std::string & name);
		Lua::LuaModule * LoadModule(const std::string & name);
		struct lua_State* GetLuaEnv() { return this->mLuaEnv; }
        bool GetFunction(const std::string & tab, const std::string & func);
	 protected:
		bool Awake() final;
		bool Start() final;
		bool LateAwake() final;
		void OnDestory() final;
		void OnLocalComplete() final;
		void OnClusterComplete() final;
        void OnSecondUpdate(const int tick) final;
        void OnHotFix() final;
    private:
		bool LoadAllFile();
		bool LoadAllFilePath(const std::string & dir);
        void AddRequire(const std::string & direct);
	 private:
		 std::string mModulePath;
		struct lua_State* mLuaEnv;
        std::set<std::string> mDirectorys;
		std::unordered_map<std::string, int> mFuncs;
		std::unordered_map<std::string, std::string> mModulePaths;
		std::unordered_map<std::string, std::unique_ptr<Lua::LuaModule>> mModules;
	};
}