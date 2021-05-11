#pragma once
#include"Manager.h"
#include<Script/ClassProxyHelper.h>
#include<Script/LuaType/LuaTable.h>
namespace SoEasy
{
	class ScriptManager : public Manager
	{
	public:
		ScriptManager();
		virtual ~ScriptManager() { }
	public:
		struct lua_State * GetLuaEnv() { return this->mLuaEnv; }
	public:
		bool LoadAllModule();
		void AddRequirePath(const std::string path);
		bool LoadLuaScript(const std::string filePath);
	public:
		template<typename T>
		bool PushGlobalPoint(const std::string name, T * data);
		inline std::string GetMainPath() { return this->mMainLuaPath; }
	protected:
		bool OnInit() override;
		void OnDestory() override;
	private:
		bool StartLoadScript();
		void OnPushGlobalObject();
		void PushClassToLua(struct lua_State * lua);
		XCode OnHotfix(long long operId);
	private:
		void RegisterExtension(struct lua_State * lua);
	private:
		std::string mMainLuaPath;
		LuaTable * mMainLuaTable;
		struct lua_State * mLuaEnv;
		const static std::string mName;
		std::vector<std::string> mRequirePaths;
	};

	template<typename T>
	inline bool ScriptManager::PushGlobalPoint(const std::string name, T * data)
	{
		const char * mateName = ClassNameProxy::GetLuaClassName<T>();
		if (mateName != nullptr)
		{
			PtrProxy<T>::Write(mLuaEnv, data);
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