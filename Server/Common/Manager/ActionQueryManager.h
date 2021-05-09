#pragma once
#include"SessionManager.h"
#include<NetWork/ActionAddressProxy.h>
#include<Protocol/ServerCommon.pb.h>

namespace SoEasy
{
	// 将本机action注册到远端 储存所有可调用action列表
	class ActionQueryManager : public SessionManager
	{
	public:
		ActionQueryManager();
		~ActionQueryManager() { }
	protected:
		bool OnInit() override;
		void OnSessionErrorAfter(shared_ptr<TcpClientSession> tcpSession) override;
		void OnSessionConnectAfter(shared_ptr<TcpClientSession> tcpSession) override;
	public:
		ActionAddressProxy * GetActionProxy(const std::string & action, long long id = 0);
	private:
		XCode UpdateActions(shared_ptr<TcpClientSession> session, long long id, const PB::AreaActionInfo & actionInfos);
	private:	
		int mAreaId;	//区服id
		std::string mQueryIp;	//查询地址的ip
		unsigned short mQueryPort;  // 查询地址的port
		class ActionManager * mActionManager;
		class ListenerManager * mListenerManager;
		class CoroutineManager * mCoroutineManager;
		shared_ptr<TcpClientSession> mActionQuerySession;
		std::unordered_map<std::string, ActionAddressProxy *> mActionProxyMap;
		std::unordered_map<std::string, std::vector<std::string>> mActionAddressMap;	//action地址
	};
}