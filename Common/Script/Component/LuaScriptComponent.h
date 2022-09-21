#pragma once

#include"Lua/WaitLuaTaskSource.h"
#include"Component/Component.h"
#include"Lua/Table.h"
#include"Lua/LocalTable.h"
#include"Lua/ClassProxyHelper.h"
namespace Sentry
{
    class LuaScriptComponent : public Component, public IStart, public IComplete, public IHotfix
	{
	 public:
		LuaScriptComponent() = default;

		virtual ~LuaScriptComponent() = default;

		struct lua_State* GetLuaEnv() { return this->mLuaEnv; }
	 protected:
		void Awake() final;
		bool OnStart() final;
        void OnHotFix() final;
		bool LateAwake() final;
		void OnDestory() final;
		void OnComplete() final;
		void OnAllServiceStart() final;
	 private:
		bool LoadAllFile();
        void AddRequire(const std::string & path);
		bool LoadLuaScript(const std::string filePath);
	 private:
		struct lua_State* mLuaEnv;
        std::set<std::string> mModules;
        std::set<std::string> mDirectorys;
        std::shared_ptr<Lua::LocalTable> mMainTable;
		std::unordered_map<std::string, std::string> mLuaFileMd5s;
	};
}