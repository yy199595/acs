#pragma once
#include"ServiceBase.h"
#include<NetWork/TcpClientSession.h>
namespace SoEasy
{
	class ProxyService : public ServiceBase
	{
	public:
		ProxyService(const std::string & name, const std::string & address, int serviceId, int areaId);
		~ProxyService() { }
	public:
		const int GetAreaId() { return this->mAreaId; }
		const std::string & GetAddress() { return this->mServiceAddress; }
		const std::string & GetServiceName() final { return mServiceName; }

	public:
		 bool OnInit() final;
		 void OnSystemUpdate() final;
	protected:
		bool HandleMessage(shared_ptr<NetWorkPacket> messageData) final;
	private:
		int mAreaId;
		const std::string mServiceName;
		const std::string mServiceAddress;
		class NetWorkManager * mNetManager;
		SharedTcpSession mProxyServiceSession;
		class ServiceManager * mServiceManager;
		std::queue<SharedPacket> mProxyMessageQueue;
	};
}