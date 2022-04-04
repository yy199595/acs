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

	 public:
		template<typename T>
		bool PushGlobalPoint(const std::string name, T* data);

	 public:
		bool GetLuaTable(const std::string& name);
		bool GetLuaFunction(const std::string& tab, const std::string& func);
	 protected:
		bool Awake() final;
		void OnStart() final;
		bool LateAwake() final;
		void OnDestory() final;
		void OnLuaRegister(lua_State * lua) final;

	 private:
		bool LoadAllFile();

		void ClearRequirePath();

		void AddRequirePath(const std::string path);

		bool LoadLuaScript(const std::string filePath);
	 private:
		struct lua_State* mLuaEnv;
		std::unordered_map<std::string, int> mGlobalRefMap;
		std::unordered_map<std::string, std::string> mLuaFileMd5s;
	};

	template<typename T>
	inline bool LuaScriptComponent::PushGlobalPoint(const std::string name, T* data)
	{
		const char* mateName = Lua::ClassNameProxy::GetLuaClassName<T>();
		if (mateName != nullptr)
		{
			Lua::PtrProxy<T>::Write(mLuaEnv, data);
			lua_getglobal(mLuaEnv, mateName);
			if (lua_istable(mLuaEnv, -1))
			{
				lua_setmetatable(mLuaEnv, -2);
				lua_setglobal(mLuaEnv, name.c_str());
				return true;
			}
		}
		return false;
	}
}