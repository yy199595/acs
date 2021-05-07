#pragma once
#include<Manager/SessionManager.h>
#include<Other/TimeRecorder.h>
#include<NetWork/TcpClientSession.h>
#include<Protocol/ServerCommon.pb.h>


namespace SoEasy
{
	// 注册本地服务， 推送本地服务到指定地址
	class ActionManager : public SessionManager
	{
	public:
		ActionManager();
		virtual ~ActionManager() { }
	public:
		void GetAllFunction(std::vector<std::string> & funcs);
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
	public:
		virtual void OnLoadLuaComplete(lua_State * luaEnv) override;
	protected:
		void OnSessionErrorAfter(shared_ptr<TcpClientSession> tcpSession) override;
		void OnSessionConnectAfter(shared_ptr<TcpClientSession> tcpSession) override;

	private:
		std::string mMessageBuffer;
		TimeRecorder mLogicTimeRecorder;
		TimeRecorder mNetLatencyRecorder;
		class ScriptManager * mScriptManager;
		class NetWorkManager * mNetWorkManager;
		class CoroutineManager * mCoroutineScheduler;
	private:
		int mAreaId;	//区服id
		std::string mQueryIp;	//查询地址的ip
		unsigned short mQueryPort;  // 查询地址的port
		shared_ptr<TcpClientSession> mActionQuerySession;
	private:
		class ListenerManager * mListenerManager;
		std::unordered_map<std::string, NetLuaAction *> mRegisterLuaActions;
		std::unordered_map<std::string, NetWorkActionBox *> mRegisterActions;
		std::unordered_map<long long, class NetWorkRetActionBox *> mRetActionMap;
	};
}