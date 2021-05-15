#include "RemoteActionProxy.h"
#include<Manager/NetWorkManager.h>
#include<Manager/SessionManager.h>
#include<Util/StringHelper.h>
namespace SoEasy
{
	RemoteActionProxy::RemoteActionProxy(const std::string name, const std::string & address, int areaId)
	{
		this->mActionName = name;
		this->mActionAreaId = areaId;
		this->mActionAddress = address;
		this->mNetWorkManager = Applocation::Get()->GetManager<NetWorkManager>();
		SayNoAssertRet_F(this->mNetWorkManager);
	}

	void RemoteActionProxy::Invoke(shared_ptr<PB::NetWorkPacket> message)
	{
		if (this->mActionSession == nullptr || !this->mActionSession->IsActive())
		{
			this->mSendQueue.push(message);
			this->mActionSession = nullptr;
			return;
		}
		this->mNetWorkManager->SendMessageByAdress(this->mActionAddress, message);
	}

	bool RemoteActionProxy::BindSession(shared_ptr<TcpClientSession> session)
	{
		if (this->mActionAddress != session->GetAddress())
		{
			return false;
		}
		this->mActionSession = session;
		while (!this->mSendQueue.empty())
		{
			shared_ptr<PB::NetWorkPacket> sendPacket = this->mSendQueue.front();
			if (!this->mNetWorkManager->SendMessageByAdress(this->mActionAddress, sendPacket))
			{
				return false;
			}
			this->mSendQueue.pop();
		}
		return true;
	}
	bool RemoteActionProxy::IsAction()
	{
		if (this->mActionSession == nullptr)
		{
			this->mActionSession = this->mNetWorkManager->GetTcpSession(this->mActionAddress);
		}
		return this->mActionSession != nullptr && this->mActionSession->IsActive();
	}
}
