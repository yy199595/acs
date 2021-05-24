#include"ProxyService.h"
#include<Manager/NetWorkManager.h>
#include<Manager/ServiceManager.h>
namespace SoEasy
{
	ProxyService::ProxyService(const std::string & name, const std::string & address, int id, int areaId)
		: mServiceAddress(address), mAreaId(areaId)
	{
		this->InitService(name, id);
	}

	bool ProxyService::OnInit()
	{
		SayNoAssertRetFalse_F(this->mNetManager = this->GetManager<NetWorkManager>());
		SayNoAssertRetFalse_F(this->mServiceManager = this->GetManager<ServiceManager>());
		return true;
	}

	void ProxyService::OnSystemUpdate()
	{
		if (this->mProxyMessageQueue.empty())
		{
			while (mProxyServiceSession->IsActive() && this->mProxyMessageQueue.empty())
			{
				SharedPacket proxyData = this->mProxyMessageQueue.front();
				this->mProxyMessageQueue.pop();
				this->mNetManager->SendMessageByAdress(this->mServiceAddress, proxyData);
			}
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
