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
	protected:
		bool InvokeMethod(const std::string & method, shared_ptr<NetWorkPacket>) final;
		bool InvokeMethod(const std::string & address, const std::string & method, SharedPacket packet) final;
	private:
		int mServiceIndex;
		lua_State * mLuaEnv;
		ScriptManager * mScriptManager;
		std::set<std::string> mMethodCacheSet;
		//std::unordered_map<std::string, int> mDefultActionMap;	//Ä¬ÈÏº¯Êý
		//std::unordered_map<std::string, NetLuaAction *> mActionMap;
	};
}