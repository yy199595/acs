#pragma once
#include"ServiceBase.h"
#include<Script/LuaInclude.h>
namespace SoEasy
{
	class ScriptManager;
	class LocalLuaService : public ServiceBase
	{
	public:
		LocalLuaService(lua_State * luaEnv, int index);
		~LocalLuaService();
	public:
		bool OnInit() final;
		bool HasMethod(const std::string & name) final;
		bool IsLuaService() final { return true; };
	protected:
		virtual XCode InvokeMethod(PB::NetWorkPacket *) final;
		virtual XCode InvokeMethod(const std::string &address, PB::NetWorkPacket *) final;
	private:
		int mServiceIndex;
		lua_State * mLuaEnv;
		ScriptManager * mScriptManager;
		std::set<std::string> mMethodCacheSet;
		//std::unordered_map<std::string, int> mDefultActionMap;	//默认函数
		//std::unordered_map<std::string, NetLuaAction *> mActionMap;
	};
}