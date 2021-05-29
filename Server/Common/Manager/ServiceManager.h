#pragma once
#include"SessionManager.h"
#include<Protocol/ServerCommon.pb.h>
namespace SoEasy
{
	class ServiceBase;
	class LocalService;
	class ProxyService;
	class LocalLuaService;
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
		LocalLuaService * AddLuaService(const std::string name, LocalLuaService * service);
		ProxyService * AddProxyService(int areaId, int serviceId, const std::string name, const std::string address);
	public:
		ProxyService * GetProxyService(int id);
		ProxyService * GetProxyService(const std::string & name);
		LocalService * GetLocalService(const std::string & name);
		LocalLuaService * GetLuaService(const std::string & name);
		SharedTcpSession GetProxySession(const std::string & address);
	public:
		bool RemoveProxyervice(const int id);
	public:
		void GetLocalServices(std::vector<ServiceBase *> & services);
	private:
		bool CreateLocalService();
	private:
		int mNodeId;
		class CoroutineManager * mCorManager;
		std::vector<std::string> mServiceList;
		std::queue<ServiceBase *> mReadyServiceQueue;
		std::unordered_map<int, ProxyService *> mProxyServiceMap;//action地址
		std::unordered_map<std::string, LocalService *> mLocalServiceMap;//action地址
		std::unordered_map<std::string, LocalLuaService *> mLuaServiceMap;	//lua服务
		std::unordered_map<std::string, shared_ptr<TcpClientSession>> mActionSessionMap; //服务通信session
	};
}