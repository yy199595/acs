#pragma once

#include"Async/Lua/WaitLuaTaskSource.h"
#include"Component/Component.h"
#include"Script/ClassProxyHelper.h"
#include"Script/Table.h"
namespace Sentry
{
	class LuaScriptComponent : public Component, public IStart, public IComplete
	{
	 public:
		LuaScriptComponent() = default;

		virtual ~LuaScriptComponent() = default;

		struct lua_State* GetLuaEnv()
		{
			return this->mLuaEnv;
		}
	 protected:
		void Awake() final;
		bool OnStart() final;
		bool LateAwake() final;
		void OnDestory() final;
		void OnComplete() final;
		void OnAllServiceStart() final;
	 private:
		bool LoadAllFile();
		bool LoadLuaScript(const std::string filePath);
	 private:
		struct lua_State* mLuaEnv;
		std::unordered_map<std::string, std::string> mLuaFileMd5s;
	};
}