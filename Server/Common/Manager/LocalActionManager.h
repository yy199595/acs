#pragma once
#include<Manager/Manager.h>
#include<Other/TimeRecorder.h>

namespace SoEasy
{
	// 注册本地服务， 推送本地服务到指定地址 管理远程回来的回调
	class NetLuaAction;
	class LocalActionProxy;
	class LocalRetActionProxy;

	class LocalActionManager : public Manager
	{
	public:
		LocalActionManager();
		virtual ~LocalActionManager() { }
	public:
		bool BindFunction(shared_ptr<NetLuaAction> actionBox);
		bool BindFunction(shared_ptr<LocalActionProxy> actionBox);
		void GetAllFunction(std::vector<std::string> & funcs);
	public:
		bool DelCallback(long long callbackId);
		bool AddCallback(shared_ptr<LocalRetActionProxy> actionBox, long long & callbackId);
		shared_ptr<LocalRetActionProxy> GetCallback(long long callbackId, bool remove = true);
	protected:
		bool OnInit() override;
		void OnDestory() override;
		void OnSecondUpdate() override;
		void OnInitComplete() override;
	public:		
		shared_ptr<NetLuaAction> GetLuaAction(const std::string & name);
		shared_ptr<LocalActionProxy> GetAction(const std::string & name);
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
		std::unordered_map<long long, shared_ptr<LocalRetActionProxy>> mRetActionMap;
		std::unordered_map<std::string, shared_ptr<NetLuaAction>> mRegisterLuaActions;
		std::unordered_map<std::string, shared_ptr<LocalActionProxy>> mRegisterActions;
	};
}