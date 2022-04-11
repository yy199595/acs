#pragma once

#include"Async/LuaTaskSource.h"
#include"Component/Component.h"
#include"Script/ClassProxyHelper.h"
#include"Script/Table.h"
namespace Sentry
{

	class ILuaRegister
	{
	 public:
		virtual void OnLuaRegister(lua_State * lua) = 0;
	};

	class LuaScriptComponent : public Component, public IStart, public ILuaRegister
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
		void OnStart() final;
		bool LateAwake() final;
		void OnDestory() final;
		void OnLuaRegister(lua_State * lua) final;

	 private:
		bool LoadAllFile();
		bool LoadLuaScript(const std::string filePath);
	 private:
		struct lua_State* mLuaEnv;
		std::unordered_map<std::string, std::string> mLuaFileMd5s;
	};
}