#include"ProxyService.h"
#include<Manager/NetWorkManager.h>
#include<Manager/ServiceManager.h>
namespace SoEasy
{
	ProxyService::ProxyService(const std::string & address, int areaId)
		: mServiceAddress(address), mAreaId(areaId)
	{
		
	}

	bool ProxyService::OnInit()
	{
		SayNoAssertRetFalse_F(this->mNetManager = this->GetManager<NetWorkManager>());
		SayNoAssertRetFalse_F(this->mServiceManager = this->GetManager<ServiceManager>());
		return ServiceBase::OnInit();
	}

	void ProxyService::OnSystemUpdate()
	{
		ServiceBase::OnSystemUpdate();
		while (!this->mProxyMessageQueue.empty() && this->mProxyServiceSession->IsActive())
		{
			SharedPacket proxyData = this->mProxyMessageQueue.front();
			this->mNetManager->SendMessageByAdress(this->mServiceAddress, proxyData);
			this->mProxyMessageQueue.pop();
		}
	}
	bool ProxyService::InvokeMethod(const std::string & method, shared_ptr<NetWorkPacket> messageData)
	{
		if (this->mProxyServiceSession == nullptr)
		{
			this->mProxyServiceSession = this->mServiceManager->GetProxySession(this->mServiceAddress);
		}
		this->mProxyMessageQueue.push(messageData);
		return true;
	}

	bool ProxyService::InvokeMethod(const std::string & address, const std::string & method, SharedPacket messageData)
	{		
		return false;
	}
}
