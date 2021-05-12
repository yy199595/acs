
#pragma once
#include"SessionManager.h"
#include<NetWork/TcpClientSession.h>
#include<Protocol/ServerCommon.pb.h>
using namespace PB;



namespace SoEasy
{
	struct ActionProxyInfo
	{
	public:
		int mAreaId;
		long long mOperId;
		std::string mAddress;
		std::string mActionName;
	public:
		bool operator == (ActionProxyInfo & actionInfo)
		{
			return this->mAreaId == actionInfo.mAreaId
				&& this->mOperId == actionInfo.mOperId
				&& this->mAddress == actionInfo.mAddress
				&& this->mActionName == actionInfo.mActionName;
		}
	};

	class TcpSessionListener;
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
		XCode RegisterActions(long long id, shared_ptr<ActionUpdateInfo> actionInfo);
		XCode QueryActions(long long id, shared_ptr<Int32Data> areaId, shared_ptr<ActionInfoList> returnData);
	private:
		void AddActionInfo(ActionProxyInfo & actionInfo);
	private:
		std::string mListenIp;
		unsigned short mListenPort;
		std::mutex mConnectSessionLock;
		class NetWorkManager * mNetWorkManager;
		std::vector<ActionProxyInfo> mActionRegisterList;
		shared_ptr<TcpSessionListener> mTcpSessionListener;
	};
}