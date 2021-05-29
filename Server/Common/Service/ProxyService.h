#pragma once
#include"ServiceBase.h"
#include<NetWork/TcpClientSession.h>
namespace SoEasy
{
	class ProxyService : public ServiceBase
	{
	public:
		ProxyService(const std::string & address, int areaId);
		~ProxyService() { }
	public:
		const int GetAreaId() { return this->mAreaId; }
		bool HasMethod(const std::string & method) final { return true; }
		const std::string & GetAddress() { return this->mServiceAddress; }
	public:
		 bool OnInit() final;
		 void OnSystemUpdate() final;
	protected:
		 bool InvokeMethod(const std::string & method, shared_ptr<NetWorkPacket>) final;
		 bool InvokeMethod(const std::string & address, const std::string & method, SharedPacket packet) final;
	private:
		int mAreaId;
		const std::string mServiceAddress;
		class NetWorkManager * mNetManager;
		SharedTcpSession mProxyServiceSession;
		class ServiceManager * mServiceManager;
		std::queue<SharedPacket> mProxyMessageQueue;
	};
}