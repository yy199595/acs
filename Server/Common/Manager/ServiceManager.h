#pragma once
#include"NetProxyManager.h"
#include<Protocol/s2s.pb.h>
#include<Other/DoubleBufferQueue.h>
namespace SoEasy
{
	class ServiceBase;
	class LocalService;
	class ProxyService;
	class LocalLuaService;
	class ServiceManager : public Manager
	{
	public:
		ServiceManager() { }
		~ServiceManager() { }
	protected:
		bool OnInit() final;
		void OnInitComplete() final;
	public:
		bool HandlerMessage(PB::NetWorkPacket * messageData);
		bool HandlerMessage(const std::string & adress, PB::NetWorkPacket * messageData);
	public:
		ServiceBase * GetService(const std::string & name);
		LocalService * GetLocalService(const std::string & name);
		LocalLuaService * GetLuaService(const std::string & name);
		LocalLuaService * AddLuaService(const std::string name, LocalLuaService * service);
	public:
		void GetLocalServices(std::vector<ServiceBase *> & services);
		void GetLocalServices(std::vector<std::string> & serviceNames);
	private:
		bool CreateLocalService();
		bool SaveRpcInfoToFile(const std::string & path);
	private:
		int mNodeId;
		class NetProxyManager * mNetManager;
		class ActionManager * mActionManager;
		class CoroutineManager * mCorManager;
		std::vector<std::string> mServiceList;
		std::unordered_map<std::string, LocalService *> mLocalServiceMap;//action地址
		std::unordered_map<std::string, LocalLuaService *> mLuaServiceMap;	//lua服务
	};
}