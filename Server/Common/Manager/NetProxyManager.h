#pragma once
#include"Manager.h"
#include<NetWork/SocketEvent.h>
#include<Other/DoubleBufferQueue.h>
namespace SoEasy
{
	// session 代理管理器 负责与网络线程通信 处理网络事件
	class NetProxyManager : public Manager
	{
	public:
		friend class NetSessionManager;
		NetProxyManager() : Manager(1) { }
		virtual ~NetProxyManager() { }
	public:
		bool AddNetSessionEvent(Net2MainEvent * eve); //不要手动调用
	public:
		bool DescorySession(const std::string & address);
		bool SendMsgByAddress(const std::string & address, PB::NetWorkPacket * msg);
		bool ConnectByAddress(const std::string & address, const std::string & name, int delayTime = 0);
	protected:
		bool OnInit() override;
		void OnSystemUpdate() final;
		virtual bool OnRecvMessage(const std::string & address, PB::NetWorkPacket * msg);
	private:
		bool HandlerNetEvent(Net2MainEvent * eve);
		bool RemoveSessionByAddress(const std::string & address);
	private:
		int mReConnectTime;
		
		class TimerManager * mTimerManager;
		class ActionManager * mActionManager;
		class ServiceManager * mServiceManager;
		class NetSessionManager * mNetWorkManager;
		std::set<std::string> mConnectSessions;		//正在连接的session
		DoubleBufferQueue<Net2MainEvent *> mNetEventQueue;
		std::unordered_map<std::string, SessionType> mSessionMap; //管理所有的session
	};
}