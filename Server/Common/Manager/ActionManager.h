#pragma once
#include<Manager/Manager.h>
#include<Other/TimeRecorder.h>

namespace SoEasy
{
	// 注册本地Lua服务，管理远程回来的回调
	class NetLuaAction;
	class LocalActionProxy;
	class LocalRetActionProxy;

	class ActionManager : public Manager
	{
	public:
		ActionManager();
		virtual ~ActionManager() { }
	public:
		bool AddCallbackArgv(SharedPacket argv);
		bool AddLuaActionArgv(const std::string & address, SharedPacket argv);
	public:
		bool BindFunction(shared_ptr<NetLuaAction> actionBox);
		bool AddCallback(shared_ptr<LocalRetActionProxy> actionBox, long long & callbackId);
	protected:
		bool OnInit() override;
		void OnDestory() override;
		void OnSystemUpdate() final;
		void OnInitComplete() override;
	public:		
		shared_ptr<NetLuaAction> GetLuaAction(const std::string & name);
		shared_ptr<LocalRetActionProxy> GetCallback(long long callbackId, bool remove = true);
	private:
		int mMessageTimeout;
		std::string mMessageBuffer;
		TimeRecorder mLogicTimeRecorder;
		TimeRecorder mNetLatencyRecorder;
		class TimerManager * mTimerManager;
		class ScriptManager * mScriptManager;
		class NetWorkManager * mNetWorkManager;
		class CoroutineManager * mCoroutineScheduler;
	private:
		DoubleBufferQueue<SharedPacket> mCallbackMessageQueue;
		std::unordered_map<long long, shared_ptr<LocalRetActionProxy>> mRetActionMap;
		std::unordered_map<std::string, shared_ptr<NetLuaAction>> mRegisterLuaActions;
	};
}