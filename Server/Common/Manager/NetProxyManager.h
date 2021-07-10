#pragma once
#include "Manager.h"
#include <NetWork/SocketEvent.h>
#include <NetWork/TcpProxySession.h>
#include <Other/DoubleBufferQueue.h>
namespace SoEasy
{
	// session 代理管理器 负责与网络线程通信 处理网络事件
	class NetProxyManager : public Manager, public ISystemUpdate
	{
	public:
		NetProxyManager() {}
		virtual ~NetProxyManager() { }

	public:
		bool AddNetSessionEvent(Net2MainEvent *eve); //不要手动调用
	public:
		bool DescorySession(const std::string &address);
		bool SendMsgByAddress(const std::string &address, PB::NetWorkPacket *msg);
		bool ConnectByAddress(const std::string &address, const std::string &name);

	public:
		TcpProxySession *GetProxySession(const std::string &address);
		TcpProxySession *DelProxySession(const std::string & address);
	protected:
		bool OnInit() override;
		void OnSystemUpdate() final;
		virtual bool OnRecvMessage(const std::string &address, PB::NetWorkPacket *msg);
	private:
		int mReConnectTime;
		class TimerManager *mTimerManager;
		class ActionManager *mActionManager;
		class ServiceManager *mServiceManager;
		class NetSessionManager *mNetWorkManager;
		DoubleBufferQueue<Net2MainEvent *> mNetEventQueue;
		std::unordered_map<std::string, TcpProxySession *> mSessionMap; //管理所有的session
		std::unordered_map<std::string, TcpProxySession *> mConnectSessionMap; //正在连接的session
	};
}