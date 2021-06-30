#pragma once
#include"SessionManager.h"
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
		void OnSystemUpdate() final;
	public:
		bool PushRequestMessage(SharedPacket messageData);
		bool PushRequestMessage(const std::string & adress, SharedPacket messageData);
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
	private:
		int mNodeId;
		class NetWorkManager * mNetManager;
		class ActionManager * mActionManager;
		class CoroutineManager * mCorManager;
		std::vector<std::string> mServiceList;
		std::queue<SharedPacket> mLocalMessageQueue;
		DoubleBufferQueue<SharedNetPacket> mRemoteMessageQueue;
		std::unordered_map<std::string, LocalService *> mLocalServiceMap;//action地址
		std::unordered_map<std::string, LocalLuaService *> mLuaServiceMap;	//lua服务
	};
}