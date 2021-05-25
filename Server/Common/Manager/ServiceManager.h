#pragma once
#include"SessionManager.h"
#include<Protocol/ServerCommon.pb.h>
namespace SoEasy
{
	class ServiceBase;
	class LocalService;
	class ProxyService;
	class ServiceManager : public SessionManager
	{
	public:
		ServiceManager() { }
		~ServiceManager() { }
	protected:
		bool OnInit() final;
		void OnInitComplete() final;
		void OnSystemUpdate() final;
		void OnSessionErrorAfter(SharedTcpSession tcpSession) final;
		void OnSessionConnectAfter(SharedTcpSession tcpSession) final;
	public:
		bool NewLocalService(const std::string & name);
	public:
		ProxyService * GetProxyService(int id);
		ProxyService * GetProxyService(const std::string & name);
		LocalService * GetLocalService(const std::string & name);
		SharedTcpSession GetProxySession(const std::string & address);
	public:
		void GetLocalServices(std::vector<LocalService *> services);
	private:
		void StartRegister();
		void QueryRemoteService();
		bool CreateLocalService();
	private:
		int mAreaId;	//区服id
		std::string mQueryIp;	//查询地址的ip
		unsigned short mQueryPort;  // 查询地址的port
		std::string mQueryAddress;
		std::string mListenAddress;
		class CoroutineManager * mCorManager;
		std::vector<std::string> mServiceList;
		class ListenerManager * mListenerManager;
		shared_ptr<TcpClientSession> mQuerySession;
		std::unordered_map<int, ProxyService *> mProxyServiceMap;//action地址
		std::unordered_map<std::string, LocalService *> mLocalServiceMap;//action地址
		std::unordered_map<std::string, shared_ptr<TcpClientSession>> mActionSessionMap; //服务通信session
	};
}