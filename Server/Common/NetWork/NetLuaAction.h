#pragma once
#include<XCode/XCode.h>
#include<Define/CommonDef.h>
#include<Protocol/Common.pb.h>
#include<Script/LuaParameter.h>
#include<NetWork/TcpClientSession.h>
using namespace PB;
namespace SoEasy
{
	class NetLuaAction
	{
	public:
		NetLuaAction(lua_State * lua, int r, const std::string name) :ref(r), luaEnv(lua), mFuncName(name) { }
		virtual ~NetLuaAction() { luaL_unref(luaEnv, LUA_REGISTRYINDEX, this->ref); }
	public:
		static NetLuaAction * Create(lua_State * luaEvn, std::string table, std::string func);
	public:
		const std::string & GetActionName() { return mFuncName; }
		XCode Invoke1(shared_ptr<TcpClientSession>, long long id);

		template<typename T>
		inline XCode Inovke2(shared_ptr<TcpClientSession>, long long id, const T & t1);

		template<typename T1, typename T2>
		inline XCode Inovke3(shared_ptr<TcpClientSession>, long long id, const T1 & requestData, T2 & returnData);

		template<typename T>
		inline XCode Inovke4(shared_ptr<TcpClientSession>, long long id, T & t1);
	public:
		XCode Invoke(shared_ptr<TcpClientSession>, const shared_ptr<NetWorkPacket> , shared_ptr<NetWorkPacket>);
	private:
		int ref;
		lua_State * luaEnv;
		std::string mFuncName;
	};
	template<typename T>
	inline XCode NetLuaAction::Inovke2(shared_ptr<TcpClientSession> session, long long id, const T & t1)
	{
		lua_rawgeti(this->luaEnv, LUA_REGISTRYINDEX, this->ref);
		if (lua_isfunction(this->luaEnv, -1))
		{
			LuaParameter::Write<shared_ptr<TcpClientSession>>(this->luaEnv, session);
			lua_pushinteger(this->luaEnv, id);
			LuaParameter::Write<const T *>(luaEnv, &t1);
			if (lua_pcall(this->luaEnv, 3, 1, 0) != 0)
			{
				const char * error = lua_tostring(luaEnv, -1);
				SayNoDebugError("call lua function error " << error);
				return XCode::CallLuaFunctionFail;
			}
			return (XCode)lua_tointeger(this->luaEnv, -1);
		}
		return XCode::CallLuaFunctionFail;
	}
	template<typename T1, typename T2>
	inline XCode NetLuaAction::Inovke3(shared_ptr<TcpClientSession> session, long long id, const T1 & t1, T2 & returnData)
	{	
		lua_rawgeti(this->luaEnv, LUA_REGISTRYINDEX, this->ref);
		if (lua_isfunction(this->luaEnv, -1))
		{
			LuaParameter::Write<shared_ptr<TcpClientSession>>(this->luaEnv, nullptr);
			lua_pushinteger(this->luaEnv, id);
			LuaParameter::Write<const T1 *>(luaEnv, &t1);
			if (lua_istable(this->luaEnv, -1))
			{				
				if (lua_pcall(this->luaEnv, 3, 2, 0) != 0)
				{
					const char * error = lua_tostring(luaEnv, -1);
					SayNoDebugError("call lua function error " << error);
					return XCode::CallLuaFunctionFail;
				}
				XCode code = (XCode)lua_tointeger(this->luaEnv, -2);
				returnData.CopyFrom(*LuaParameter::Read<T2 *>(this->luaEnv, -1));		
				return code;
			}
		}
		return XCode::CallLuaFunctionFail;
	}
	template<typename T>
	inline XCode NetLuaAction::Inovke4(shared_ptr<TcpClientSession> session, long long id, T & returnData)
	{
		lua_rawgeti(this->luaEnv, LUA_REGISTRYINDEX, this->ref);
		if (lua_isfunction(this->luaEnv, -1))
		{
			LuaParameter::Write<shared_ptr<TcpClientSession>>(this->luaEnv, session);
			lua_pushinteger(this->luaEnv, id);
			if (lua_pcall(this->luaEnv, 2, 2, 0) != 0)
			{
				const char * error = lua_tostring(luaEnv, -1);
				SayNoDebugError("call lua function error " << error);
				return XCode::CallLuaFunctionFail;
			}
			XCode code = (XCode)lua_tointeger(this->luaEnv, -2);
			returnData.CopyFrom(*LuaParameter::Read<T *>(this->luaEnv, -1));
			return code;
		}
		return XCode::CallLuaFunctionFail;
	}
}
