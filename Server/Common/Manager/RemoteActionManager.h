#pragma once
#include"SessionManager.h"
#include<NetWork/RemoteActionProxy.h>
#include<Protocol/ServerCommon.pb.h>

namespace SoEasy
{
	// 将本机action注册到远端 储存所有可调用action列表

	class RemoteActionManager : public SessionManager
	{
	public:
		RemoteActionManager();
		~RemoteActionManager() { }
	protected:
		bool OnInit() override;
		void OnSessionErrorAfter(shared_ptr<TcpClientSession> tcpSession) override;
		void OnSessionConnectAfter(shared_ptr<TcpClientSession> tcpSession) override;
	public:
		bool GetActionProxy(const std::string & action, shared_ptr<RemoteActionProxy> & actionProxy);
		bool GetActionProxy(const std::string & action, std::vector<shared_ptr<RemoteActionProxy>> & actionProxys);
		bool GetActionProxy(const std::string & action, long long operId, shared_ptr<RemoteActionProxy> & actionProxy);
		void GetActionProxyByAddress(const std::string & address, std::vector<shared_ptr<RemoteActionProxy>> & actionProxys);
	private:
		void StartRegisterAction();
		void StartPullActionList();  //开始拉取action列表
		bool StartConnectToAction(shared_ptr<RemoteActionProxy> actionProxy);
		void AddNewActionProxy(int argaId, const std::string & name, const std::string & address);
	private:	
		int mAreaId;	//区服id
		std::string mQueryIp;	//查询地址的ip
		unsigned short mQueryPort;  // 查询地址的port
		std::string mQueryAddress;
		class LocalActionManager * mActionManager;
		class ListenerManager * mListenerManager;
		class CoroutineManager * mCoroutineManager;
		shared_ptr<TcpClientSession> mActionQuerySession;
		std::unordered_map<std::string, shared_ptr<TcpClientSession>> mActionSessionMap;
		std::unordered_map<std::string, std::vector<shared_ptr<RemoteActionProxy>>> mActionProxyMap;//action地址
	};
}