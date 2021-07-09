#pragma once

#include<XCode/XCode.h>
#include<Object/Object.h>
#include<Script/LuaInclude.h>
#include<Global/ServerConfig.h>
#include<Other/DoubleBufferQueue.h>
#include<NetWork/TcpClientSession.h>
#include<Protocol/com.pb.h>
#include"ManagerInterface.h"
using namespace PB;
namespace SoEasy
{
	typedef shared_ptr<NetWorkPacket> SharedPacket;
	struct NetMessageBuffer
	{
	public:
		NetMessageBuffer(const std::string & address, const SharedPacket packet);
	public:
		const std::string mAddress;
		const SharedPacket mMessagePacket;
	};
	
	typedef shared_ptr<NetMessageBuffer> SharedNetPacket;

	class ThreadTaskAction;
	class LocalActionProxy;
	
	class Manager : public Object
	{
	public:
		Manager(const int priority = 10);
		virtual ~Manager() { }
	protected:
		friend Applocation;
	public:
		void AddFinishTask(long long id);	//不要手动调用
	public:
		bool StartTaskAction(shared_ptr<ThreadTaskAction> taskAction);
	public:
		inline int GetPriority() { return mPriority; }	//优先级(根据优先级确定调用顺序)
		virtual void PushClassToLua(lua_State * luaEnv) { }		//自身方法导出到lua
	protected:
		void ForeachManagers(std::function<bool(Manager *)> action);
	protected:
		virtual bool OnInit() = 0;						//初始化管理器
		virtual void OnHitfix() { }						//热更命令之后调用
		virtual void OnInitComplete() { }				//在初始化完成之后 改方法会在协程中调用
		virtual void OnSystemUpdate();					//处理系统事件
		virtual void OnFrameUpdate(float t) { }			//逻辑帧
		virtual void OnSecondUpdate() { }				//每秒调用
		virtual void OnFrameUpdateAfter() { }			//在逻辑帧执行完成之后
		virtual void OnTaskFinish(shared_ptr<ThreadTaskAction> taskAction) { }		//在线程池完成任务之后的通知	
	private:
		const int mPriority;
	};
	
	

	inline std::string GetFunctionName(std::string func)
	{
		return func.substr(func.find("::") + 2);
	}

}