#include"SystemExtension.h"
#include<Util/TimeHelper.h>
#include<Core/Applocation.h>
#include<Manager/TimerManager.h>
#include<NetWork/RemoteScheduler.h>
#include<Timer/LuaActionTimer.h>
#include<Timer/LuaSleepTimer.h>
using namespace SoEasy;
namespace SystemExtension
{
	int Call(lua_State * lua)
	{
		shared_ptr<TcpClientSession> tcpSession = LuaParameter::Read<shared_ptr<TcpClientSession>>(lua, 1);
		if (tcpSession == nullptr)
		{
			lua_pushboolean(lua, false);
			return 1;
		}

		const char * funcName = lua_tostring(lua, 2);
		RemoteScheduler nCallController(tcpSession);

		if (lua_isfunction(lua, 3))
		{
			std::string message = "";
			NetLuaRetAction * pRetAction = NetLuaRetAction::Create(lua, 3);
			lua_pushboolean(lua, nCallController.Call(funcName, nullptr, pRetAction));
			return 1;
		}
		else if (lua_isuserdata(lua, 3))
		{
			Message * pMessage = (Message *)lua_touserdata(lua, 3);
			if (pMessage != nullptr)
			{
				if (!lua_isfunction(lua, 4))
				{
					lua_pushboolean(lua, nCallController.Call(funcName, pMessage));
					return 1;
				}
				NetLuaRetAction * pRetAction = NetLuaRetAction::Create(lua, 4);
				lua_pushboolean(lua, nCallController.Call(funcName, pMessage, pRetAction));
				return 1;
			}
		}
		else if (lua_istable(lua, 3))
		{			
			LuaTable luaTable(lua, 3, true);
			if (lua_isfunction(lua, 4))
			{
				NetLuaRetAction * pRetAction = NetLuaRetAction::Create(lua, 4);
				lua_pushboolean(lua, nCallController.Call(funcName, luaTable, pRetAction));
				return 1;
			}
			lua_pushboolean(lua, nCallController.Call(funcName, luaTable));
			return 1;
		}
		else
		{
			lua_pushboolean(lua, nCallController.Call(funcName));
			return 1;
		}

		lua_pushboolean(lua, false);
		return 1;
	}

	int GetApp(lua_State * luaEnv)
	{
		Applocation * pApplocation = Applocation::Get();
		LuaParameter::Write<Applocation *>(luaEnv, pApplocation);
		return 1;
	}

	int GetManager(lua_State * luaEnv)
	{
		if (lua_isstring(luaEnv, -1))
		{
			const std::string name = lua_tostring(luaEnv, -1);
			//TODO
			PtrProxy<Manager>::Write(luaEnv, nullptr);
			if (lua_getglobal(luaEnv, name.c_str()) && lua_istable(luaEnv, -1))
			{
				lua_setmetatable(luaEnv, -2);
				return 1;
			}
		}
		return 0;
	}

	int CallWait(lua_State * luaEnv)
	{
		if (!lua_isuserdata(luaEnv, 2))
		{
			lua_pushnil(luaEnv);
			return 1;
		}
		if (!lua_isfunction(luaEnv, 1))
		{
			lua_pushnil(luaEnv);
			return 1;
		}
		lua_State * coroutine = lua_newthread(luaEnv);
		lua_pushvalue(luaEnv, 1);
		lua_xmove(luaEnv, coroutine, 1);
		lua_replace(luaEnv, 1);

		const int size = lua_gettop(luaEnv);
		lua_xmove(luaEnv, coroutine, size - 1);
		
		lua_resume(coroutine, luaEnv, 1);
		return 1;
	}

	int WaitFor(lua_State * luaEnv)
	{
		shared_ptr<TcpClientSession> tcpSession = LuaParameter::Read<shared_ptr<TcpClientSession>>(luaEnv, 1);
		if (tcpSession == nullptr)
		{
			lua_pushinteger(luaEnv, XCode::Failure);
			return 1;
		}
		lua_pushthread(luaEnv);
		if (!lua_isthread(luaEnv, -1))
		{
			lua_pushinteger(luaEnv, XCode::Failure);
			return 1;
		}

		NetLuaWaitAction * waitAction = NetLuaWaitAction::Create(luaEnv, -1);
		if (waitAction == nullptr)
		{
			lua_pushinteger(luaEnv, XCode::Failure);
			return 1;
		}

		RemoteScheduler nCallController(tcpSession);
		const char * funcName = lua_tostring(luaEnv, 2);
		if (lua_isuserdata(luaEnv, 3))
		{
			std::string nMessageBuffer;
			Message * pMessage = (Message*)lua_touserdata(luaEnv, 3);
			if (!pMessage->SerializePartialToString(&nMessageBuffer))
			{
				lua_pushinteger(luaEnv, XCode::SerializationFailure);
				return 1;
			}
			if (!nCallController.Call(funcName, pMessage, waitAction))
			{
				lua_pushinteger(luaEnv, XCode::Failure);
				return 1;
			}
		}
		else if (lua_istable(luaEnv, 3))
		{
			LuaTable luaTable(luaEnv, 3, true);
			if (!nCallController.Call(funcName, luaTable, waitAction))
			{
				lua_pushinteger(luaEnv, XCode::Failure);
				return 1;
			}
		}
		else
		{
			if (!nCallController.Call(funcName, nullptr, waitAction))
			{
				lua_pushinteger(luaEnv, XCode::Failure);
				return 1;
			}
		}
		return lua_yield(luaEnv, 1);
	}

	int Start(lua_State * luaEnv)
	{
		lua_State * coroutine = lua_newthread(luaEnv);
		lua_pushvalue(luaEnv, 1);
		lua_xmove(luaEnv, coroutine, 1);
		lua_replace(luaEnv, 1);

		const int size = lua_gettop(luaEnv);
		lua_xmove(luaEnv, coroutine, size - 1);

		return lua_resume(coroutine, luaEnv, 1);
	}

	int Sleep(lua_State * luaEnv)
	{	
		long long ms = lua_tointeger(luaEnv, 1);
		lua_pushthread(luaEnv);
		//TODO
		TimerManager * pTimerManager = nullptr;
		if (pTimerManager != nullptr)
		{
			long long targetTime = TimeHelper::GetMilTimestamp() + ms;
			shared_ptr<TimerBase> pTimer = LuaSleepTimer::Create(luaEnv, -1, targetTime);
			pTimerManager->AddTimer(pTimer);
		}
		return lua_yield(luaEnv, 01);
	}

	int WaitNetFrame(lua_State * luaEnv)
	{
		lua_pushthread(luaEnv);
		int ref = luaL_ref(luaEnv, LUA_REGISTRYINDEX);
		lua_State * coroutine = lua_tothread(luaEnv, -1);
		return lua_yield(coroutine, 0);
	}

	int AddTimer(lua_State * lua)
	{
		if (!lua_isfunction(lua, 1))
		{
			return 0;
		}
		if (!lua_isinteger(lua, 2))
		{
			return 0;
		}
		int count = 1;
		int interval = lua_tointeger(lua, 2);
		if (lua_isinteger(lua, 3))
		{
			count = lua_tointeger(lua, 3);
		}
		//TODO
		TimerManager * pTimerManager = nullptr;
		if (pTimerManager != nullptr)
		{
			lua_pushvalue(lua, 1);
			if (!lua_isfunction(lua, -1))
			{
				return 0;
			}
			int ref = luaL_ref(lua, LUA_REGISTRYINDEX);
			shared_ptr<LuaActionTimer> sharedTimer = pTimerManager->CreateTimer<LuaActionTimer>(lua, ref, interval, count);
			if (sharedTimer != nullptr)
			{
				lua_pushinteger(lua, sharedTimer->GetTimerId());
				return 1;
			}		
		}
		return 0;
	}
	int RemoveTimer(lua_State * lua)
	{
		if (lua_isinteger(lua, 1))
		{
			long long id = lua_tointeger(lua, 1);
			//TODO
			TimerManager * pTimerManager = nullptr;
			if (pTimerManager != nullptr)
			{
				bool code = pTimerManager->RemoveTimer(id);
				lua_pushboolean(lua, code);
				return 1;
			}
		}	
		lua_pushboolean(lua, false);
		return 1;
	}
}