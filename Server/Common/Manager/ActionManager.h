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
		long long AddCallback(shared_ptr<LocalRetActionProxy> rpcAction);
		bool AddActionArgv(long long id, shared_ptr<NetWorkPacket> messageData);	
	protected:
		bool OnInit() override;
		void OnSystemUpdate() final;					//处理系统事件
	private:
		int mMessageTimeout;
		std::string mMessageBuffer;
		TimeRecorder mLogicTimeRecorder;
		TimeRecorder mNetLatencyRecorder;
		class TimerManager * mTimerManager;
	private:
		DoubleBufferQueue<SharedPacket> mCallbackMessageQueue;
		std::unordered_map<long long, shared_ptr<LocalRetActionProxy>> mRetActionMap;
	};
}