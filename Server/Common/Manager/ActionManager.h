#pragma once
#include<Manager/Manager.h>
#include<Other/TimeRecorder.h>

namespace SoEasy
{
	// 注册本地服务， 推送本地服务到指定地址 管理远程回来的回调
	class ActionManager : public Manager
	{
	public:
		ActionManager();
		virtual ~ActionManager() { }
	public:
		void GetAllFunction(std::vector<std::string> & funcs);

		bool BindFunction(class NetLuaAction * actionBox);
		bool BindFunction(class NetWorkActionBox * actionBox);
		long long AddCallback(class NetWorkRetActionBox * actionBox);
	protected:
		bool OnInit() override;
		void OnDestory() override;
		void OnSecondUpdate() override;
		void OnInitComplete() override;
	public:
		bool Call(shared_ptr<TcpClientSession> tcpSession, const long long id, const shared_ptr<NetWorkPacket> callInfo);
		bool Call(shared_ptr<TcpClientSession> tcpSession, const std::string & name, const shared_ptr<NetWorkPacket> callInfo);
	private:		
		NetLuaAction * GetLuaFunction(const std::string & name);
		NetWorkActionBox * GetFunction(const std::string & name);
	private:
		std::string mMessageBuffer;
		TimeRecorder mLogicTimeRecorder;
		TimeRecorder mNetLatencyRecorder;
		class ScriptManager * mScriptManager;
		class NetWorkManager * mNetWorkManager;
		class CoroutineManager * mCoroutineScheduler;
	private:
		std::unordered_map<std::string, NetLuaAction *> mRegisterLuaActions;
		std::unordered_map<std::string, NetWorkActionBox *> mRegisterActions;
		std::unordered_map<long long, class NetWorkRetActionBox *> mRetActionMap;
	};
}