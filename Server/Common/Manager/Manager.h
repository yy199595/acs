#pragma once

#include<XCode/XCode.h>
#include<Object/Object.h>
#include<Script/LuaInclude.h>
#include<Global/ServerConfig.h>
#include<Other/DoubleBufferQueue.h>
#include<NetWork/TcpClientSession.h>
#include<Protocol/com.pb.h>
#include"ManagerInterface.h"
using namespace com;
namespace Sentry
{
	class ThreadTaskAction;
	class LocalActionProxy;
	
	class Manager : public Object
	{
	public:
		Manager() { }
		virtual ~Manager() { }
	protected:
		friend Applocation;
	public:
		virtual void PushClassToLua(lua_State * luaEnv) { }		//自身方法导出到lua
	protected:
		void ForeachManagers(std::function<bool(Manager *)> action);
	protected:
		virtual bool OnInit() = 0;						//初始化管理器
		virtual void OnHitfix() { }						//热更命令之后调用
		virtual void OnInitComplete() { }				//在初始化完成之后 改方法会在协程中调用
	};

	inline std::string GetFunctionName(std::string func)
	{
		return func.substr(func.find("::") + 2);
	}

}