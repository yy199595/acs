#pragma once
#include"SessionManager.h"
#include<Protocol/s2s.pb.h>
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
		LocalService * GetLocalService(const std::string & name);
		LocalLuaService * GetLuaService(const std::string & name);
		LocalLuaService * AddLuaService(const std::string name, LocalLuaService * service);
	public:
		void GetLocalServices(std::vector<std::string> & serviceNames);
	private:
		bool CreateLocalService();
	private:
		int mNodeId;
		class CoroutineManager * mCorManager;
		std::vector<std::string> mServiceList;
		std::vector<ServiceBase *> mServiceVector;
		std::unordered_map<std::string, LocalService *> mLocalServiceMap;//action地址
		std::unordered_map<std::string, LocalLuaService *> mLuaServiceMap;	//lua服务
		std::unordered_map<std::string, shared_ptr<TcpClientSession>> mActionSessionMap; //服务通信session
	};
}