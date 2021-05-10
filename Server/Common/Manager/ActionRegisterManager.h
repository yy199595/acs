
#pragma once
#include"SessionManager.h"
#include<Other/AreaActionTable.h>
#include<NetWork/TcpClientSession.h>
#include<Protocol/ServerCommon.pb.h>
using namespace PB;



namespace SoEasy
{
	// 所有方法都注册到这里(全局唯一)
	class ActionRegisterManager : public SessionManager
	{
	public:
		ActionRegisterManager() { }
		~ActionRegisterManager() { }
	protected:
		bool OnInit() override;
		void OnSecondUpdate() override;
		void OnInitComplete() override;
		void OnSessionErrorAfter(shared_ptr<TcpClientSession> tcpSession) override;
		void OnSessionConnectAfter(shared_ptr<TcpClientSession> tcpSession) override;
	private:
		XCode RegisterActions(shared_ptr<TcpClientSession> session, long long id, shared_ptr<ActionUpdateInfo> actionInfo);
	private:
		XCode SyncActionInfos(int areaId);
	private:
		std::string mListenIp;
		unsigned short mListenPort;
		std::mutex mConnectSessionLock;
		class NetWorkManager * mNetWorkManager;
		shared_ptr<TcpSessionListener> mTcpSessionListener;
		std::unordered_map<std::string, int> mAddressAreaIdMap;
		std::unordered_map<int, AreaActionTable *> mAreaActionMap;
	};
}