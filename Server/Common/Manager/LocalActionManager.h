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
		bool BindFunction( NetLuaAction * actionBox);
		bool BindFunction( LocalActionProxy * actionBox);
		void GetAllFunction(std::vector<std::string> & funcs);
	public:
		bool DelCallback(long long callbackId);
		LocalRetActionProxy * GetCallback(long long callbackId, bool remove = true);
		bool AddCallback(LocalRetActionProxy * actionBox, long long & callbackId);
	protected:
		bool OnInit() override;
		void OnDestory() override;
		void OnSecondUpdate() override;
		void OnInitComplete() override;
	/*public:
		bool Call(shared_ptr<TcpClientSession> tcpSession, const long long id, const shared_ptr<NetWorkPacket> callInfo);
		bool Call(shared_ptr<TcpClientSession> tcpSession, const std::string & name, const shared_ptr<NetWorkPacket> callInfo);*/
	public:		
		NetLuaAction * GetLuaAction(const std::string & name);
		LocalActionProxy * GetAction(const std::string & name);
	private:
		std::string mMessageBuffer;
		TimeRecorder mLogicTimeRecorder;
		TimeRecorder mNetLatencyRecorder;
		class ScriptManager * mScriptManager;
		class NetWorkManager * mNetWorkManager;
		class CoroutineManager * mCoroutineScheduler;
	private:
		std::unordered_map<long long, LocalRetActionProxy *> mRetActionMap;
		std::unordered_map<std::string, NetLuaAction *> mRegisterLuaActions;
		std::unordered_map<std::string, LocalActionProxy *> mRegisterActions;
	};
}