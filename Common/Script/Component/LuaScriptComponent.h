#pragma once

#include"Lua/WaitLuaTaskSource.h"
#include"Component/Component.h"
#include"Lua/Table.h"
#include"Lua/LocalTable.h"
#include"Lua/ClassProxyHelper.h"
namespace Sentry
{
    class LuaScriptComponent : public Component, public IStart, public IComplete, public IHotfix, public IServiceChange
	{
	 public:
		LuaScriptComponent() = default;
		virtual ~LuaScriptComponent() = default;
    public:
        bool LoadModule(const std::string & name);
        struct lua_State* GetLuaEnv() { return this->mLuaEnv; }
        bool GetFunction(const std::string & tab, const std::string & func);
	 protected:
		bool Awake() final;
		bool Start() final;
        void OnHotFix() final;
		bool LateAwake() final;
		void OnDestory() final;
		void OnLocalComplete() final;
		void OnClusterComplete() final;
        bool OnStartService(const std::string &name) final;
        bool OnCloseService(const std::string &name) final;
    private:
		bool LoadAllFile();
        void AddRequire(const std::string & direct);
	 private:
		struct lua_State* mLuaEnv;
        std::set<std::string> mModules;
        std::set<std::string> mDirectorys;
		std::unordered_map<std::string, std::string> mModulePaths;
	};
}