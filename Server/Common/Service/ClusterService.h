#pragma once
#include"LocalService.h"
#include<Protocol/Common.pb.h>
#include<Protocol/ServerCommon.pb.h>
using namespace PB;
namespace SoEasy
{
	class ServiceManager;
	class CoroutineManager;
	class ClusterService : public LocalService
	{
	public:
		ClusterService() { }
		~ClusterService() { }
	public:
		bool OnInit() final;
		void OnInitComplete()final;
		void OnConnectDone(SharedTcpSession tcpSession) final;
	private:
		void StarRegister();
	private:
		XCode RemoveService(long long, shared_ptr<Int32Data> service);
		XCode RefreshServices(long long, shared_ptr<ServicesNotice> services);
	private:
		unsigned short mAreaId;
		unsigned short mNodeId;
		std::string mQueryAddress;
		std::string mListenAddress;
		ServiceManager * mServiceManager;
		class ProxyService * mRegistryService;
	};
}