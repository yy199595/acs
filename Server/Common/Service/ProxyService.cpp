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
		if (!this->mProxyMessageQueue.empty() && mProxyServiceSession->IsActive())
		{
			SharedPacket proxyData = this->mProxyMessageQueue.front();
			this->mProxyMessageQueue.pop();
			this->mNetManager->SendMessageByAdress(this->mServiceAddress, proxyData);
		}
	}
	bool ProxyService::HandleMessage(shared_ptr<NetWorkPacket> messageData)
	{
		this->mProxyMessageQueue.push(messageData);
		if (this->mProxyServiceSession == nullptr)
		{
			this->mProxyServiceSession = this->mServiceManager->GetProxySession(this->mServiceAddress);
			SayNoAssertRetFalse_F(this->mProxyServiceSession);
		}
		return true;
	}
}
