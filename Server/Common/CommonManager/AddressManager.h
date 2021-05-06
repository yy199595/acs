
#pragma once
#include"SessionManager.h"

#include<CommonOther/AreaActionTable.h>
#include<CommonNetWork/TcpClientSession.h>
#include<CommonProtocol/ServerCommon.pb.h>
using namespace PB;



namespace SoEasy
{
	// 查询 调用方法地址
	class AddressManager : public SessionManager
	{
	public:
		AddressManager() { }
		~AddressManager() { }
	protected:
		bool OnInit() override;
		void OnSecondUpdate() override;
		void OnInitComplete() override;
		void OnSessionErrorAfter(shared_ptr<TcpClientSession> tcpSession) override;
		void OnSessionConnectAfter(shared_ptr<TcpClientSession> tcpSession) override;
	private:
		XCode UpdateActionAddress(shared_ptr<TcpClientSession> session, long long id, const ActionUpdateInfo & actionInfo);
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