#pragma once

#include"Async/LuaTaskSource.h"
#include"Component/Component.h"
#include"Script/ClassProxyHelper.h"
#include"Script/Table.h"
namespace Sentry
{
	class LuaScriptComponent : public Component, public IStart
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
	 private:
		bool LoadAllFile();
		bool LoadSoFile(const std::string & path);
		bool LoadLuaScript(const std::string filePath);
	 private:
		struct lua_State* mLuaEnv;
		std::unordered_map<std::string, std::string> mLuaFileMd5s;
	};
}