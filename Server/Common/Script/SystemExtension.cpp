#include"SystemExtension.h"
#include<Util/TimeHelper.h>
#include<Core/Applocation.h>
#include<Manager/TimerManager.h>
#include<NetWork/RemoteScheduler.h>
#include<Timer/LuaActionTimer.h>
#include<Timer/LuaSleepTimer.h>
#include<Manager/NetWorkManager.h>
#include<Manager/ActionManager.h>
#include<Protocol/Common.pb.h>
#include<NetWork/NetLuaRetAction.h>
#include<Service/LocalLuaService.h>
#include<Manager/ServiceManager.h>
using namespace SoEasy;

	int SystemExtension::Call(lua_State * lua)
	{
		shared_ptr<TcpClientSession> tcpSession = LuaParameter::Read<shared_ptr<TcpClientSession>>(lua, 1);
		if (tcpSession == nullptr)
		{
			lua_pushboolean(lua, false);
			return 1;
		}

		const char * action = lua_tostring(lua, 3);
		const char * service = lua_tostring(lua, 2);
		RemoteScheduler nCallController(tcpSession);

		if (lua_isfunction(lua, 4))
		{
			std::string message = "";
			NetLuaRetAction * pRetAction = NetLuaRetAction::Create(lua, 4);
			lua_pushboolean(lua, nCallController.Call(service, action, nullptr, pRetAction));
			return 1;
		}
		else if (lua_isuserdata(lua, 4))
		{
			Message * pMessage = (Message *)lua_touserdata(lua, 4);
			if (pMessage != nullptr)
			{
				if (!lua_isfunction(lua, 5))
				{
					lua_pushboolean(lua, nCallController.Call(service, action, pMessage));
					return 1;
				}
				NetLuaRetAction * pRetAction = NetLuaRetAction::Create(lua, 5);
				lua_pushboolean(lua, nCallController.Call(service, action, pMessage, pRetAction));
				return 1;
			}
		}
		else if (lua_istable(lua, 4))
		{
			LuaTable luaTable(lua, 4, true);
			if (lua_isfunction(lua, 5))
			{
				NetLuaRetAction * pRetAction = NetLuaRetAction::Create(lua, 5);
				lua_pushboolean(lua, nCallController.Call(service, action, luaTable, pRetAction));
				return 1;
			}
			lua_pushboolean(lua, nCallController.Call(service, action, luaTable));
			return 1;
		}
		else
		{
			lua_pushboolean(lua, nCallController.Call(service, action));
			return 1;
		}

		lua_pushboolean(lua, false);
		return 1;
	}

	int SystemExtension::GetApp(lua_State * luaEnv)
	{
		Applocation * pApplocation = Applocation::Get();
		LuaParameter::Write<Applocation *>(luaEnv, pApplocation);
		return 1;
	}

	int SystemExtension::GetManager(lua_State * luaEnv)
	{
		if (lua_isstring(luaEnv, -1))
		{
			Applocation * app = Applocation::Get();
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

	extern bool SystemExtension::RequireLua(lua_State * luaEnv, const char * name)
	{
		lua_getglobal(luaEnv, "require");
		if (lua_isfunction(luaEnv, -1))
		{
			lua_pushstring(luaEnv, name);
			if (lua_pcall(luaEnv, 1, 1, 0) != 0)
			{
				SayNoDebugError(lua_tostring(luaEnv, -1));
				return false;
			}
			return lua_istable(luaEnv, -1);
		}
		return false;
	}

	int SystemExtension::NewService(lua_State * luaEnv)
	{
		const char * name = lua_tostring(luaEnv, 1);
		if (SystemExtension::RequireLua(luaEnv, name))
		{
			Applocation * app = Applocation::Get();
			LocalLuaService * luaService = new LocalLuaService(luaEnv, -1);
			luaService->Init(app, name);
			ServiceManager * serviceMgr = app->GetManager<ServiceManager>();
			if (serviceMgr != nullptr)
			{
				if (serviceMgr->AddLuaService(name, luaService) != nullptr)
				{
					int id = luaService->GetServiceId();
					lua_pushinteger(luaEnv, id);
					return 1;
				}
			}
		}
		return 0;
	}

	int SystemExtension::LuaRetMessage(lua_State * luaEnv)
	{
		Applocation * app = Applocation::Get();
		if (lua_isstring(luaEnv, 1)) //远程回复
		{		
			NetWorkManager * netManager = app->GetManager<NetWorkManager>();
			if (netManager != nullptr)
			{
				const std::string address = lua_tostring(luaEnv, 1);
				const long long callbackId = lua_tointeger(luaEnv, 2);
				const long long operId = lua_tointeger(luaEnv, 3);
				const int code = lua_tointeger(luaEnv, 4);
				shared_ptr<PB::NetWorkPacket> returnPacket = make_shared<NetWorkPacket>();

				returnPacket->set_error_code(code);
				returnPacket->set_operator_id(operId);
				returnPacket->set_callback_id(callbackId);
				if (lua_isstring(luaEnv, 5))
				{
					size_t size = 0;
					const char * str = lua_tolstring(luaEnv, 5, &size);
					returnPacket->set_message_data(str, size);
				}
				netManager->SendMessageByAdress(address, returnPacket);
				return 0;
			}
		}
		else if (lua_isinteger(luaEnv, 1)) //本机回复
		{
			ActionManager * actManager = app->GetManager<ActionManager>();
			
			const long long callbackId = lua_tointeger(luaEnv, 1);
			const long long operId = lua_tointeger(luaEnv, 2);
			const int code = lua_tointeger(luaEnv, 3);
			
			shared_ptr<PB::NetWorkPacket> returnPacket = make_shared<NetWorkPacket>();

			returnPacket->set_error_code(code);
			returnPacket->set_operator_id(operId);
			returnPacket->set_callback_id(callbackId);
			if (lua_isstring(luaEnv, 4))
			{
				size_t size = 0;
				const char * str = lua_tolstring(luaEnv, 4, &size);
				returnPacket->set_message_data(str, size);
			}
			actManager->AddActionArgv(callbackId, returnPacket);
		}
		return 0;
	}

	int SystemExtension::CallWait(lua_State * luaEnv)
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

	int SystemExtension::CallByName(lua_State * luaEnv)
	{
		/*lua_pushthread(luaEnv);
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

		RemoteScheduler nCallController;
		const char * action = lua_tostring(luaEnv, 3);
		const char * service = lua_tostring(luaEnv, 2);
		if (lua_isuserdata(luaEnv, 4))
		{
			std::string nMessageBuffer;
			Message * pMessage = (Message*)lua_touserdata(luaEnv, 4);
			if (!pMessage->SerializePartialToString(&nMessageBuffer))
			{
				lua_pushinteger(luaEnv, XCode::SerializationFailure);
				return 1;
			}
			if (!nCallController.Call(service, action, pMessage, waitAction))
			{
				lua_pushinteger(luaEnv, XCode::Failure);
				return 1;
			}
		}
		else if (lua_istable(luaEnv, 4))
		{
			LuaTable luaTable(luaEnv, 4, true);
			if (!nCallController.Call(service, action, luaTable, waitAction))
			{
				lua_pushinteger(luaEnv, XCode::Failure);
				return 1;
			}
		}
		else
		{
			if (!nCallController.Call(service, action, nullptr, waitAction))
			{
				lua_pushinteger(luaEnv, XCode::Failure);
				return 1;
			}
		}*/
		return lua_yield(luaEnv, 1);
	}

	int SystemExtension::CallBySession(lua_State * luaEnv)
	{
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
		SharedTcpSession tcpSession = LuaParameter::Read<SharedTcpSession>(luaEnv, 1);
		RemoteScheduler nCallController(tcpSession);	
		const char * action = lua_tostring(luaEnv, 3);
		const char * service = lua_tostring(luaEnv, 2);
		if (lua_isuserdata(luaEnv, 4))
		{
			Message * pMessage = (Message*)lua_touserdata(luaEnv, 4);
			XCode code = nCallController.Call(service, action, pMessage, waitAction);
			if (code != XCode::Successful)
			{
				lua_pushinteger(luaEnv, code);
				return 1;
			}
		}
		else if (lua_istable(luaEnv, 4))
		{
			LuaTable luaTable(luaEnv, 4, true);
			XCode code = nCallController.Call(service, action, luaTable, waitAction);
			if (code != XCode::Successful)
			{
				lua_pushinteger(luaEnv, code);
				return 1;
			}
		}
		else
		{
			XCode code = nCallController.Call(service, action, nullptr, waitAction);
			if (code != XCode::Successful)
			{
				lua_pushinteger(luaEnv, code);
				return 1;
			}
		}
		return lua_yield(luaEnv, 1);
	}

	int SystemExtension::Sleep(lua_State * luaEnv)
	{
		long long ms = lua_tointeger(luaEnv, 1);
		lua_pushthread(luaEnv);
		TimerManager * pTimerManager = Applocation::Get()->GetManager<TimerManager>();
		if (pTimerManager != nullptr)
		{
			shared_ptr<TimerBase> pTimer = LuaSleepTimer::Create(luaEnv, -1, ms);
			pTimerManager->AddTimer(pTimer);
		}
		return lua_yield(luaEnv, 1);
	}

	int SystemExtension::AddTimer(lua_State * lua)
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
		Applocation * app = Applocation::Get();
		TimerManager * pTimerManager = app->GetManager<TimerManager>();
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
	int SystemExtension::RemoveTimer(lua_State * lua)
	{
		if (lua_isinteger(lua, 1))
		{
			long long id = lua_tointeger(lua, 1);
			Applocation * app = Applocation::Get();
			TimerManager * pTimerManager = app->GetManager<TimerManager>();
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
