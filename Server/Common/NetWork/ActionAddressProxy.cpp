#include "ActionAddressProxy.h"
#include<Manager/NetWorkManager.h>
#include<Manager/SessionManager.h>
#include<Util/StringHelper.h>
namespace SoEasy
{

	ActionAddressProxy::ActionAddressProxy(NetWorkManager * mgr, SessionManager * sMgr, const std::string & address)
	{
		this->mNetWorkManager = mgr;
		this->mSessionManager = sMgr;
		this->mActionAddress = address;
		SayNoAssertRet_F(this->mSessionManager);
		SayNoAssertRet_F(this->mNetWorkManager);
		SayNoAssertRet_F(StringHelper::ParseIpAddress(address, this->mActionIp, this->mActionPort));
		mActionSession = make_shared<TcpClientSession>(this->mSessionManager, "ActionSession", this->mActionIp, this->mActionPort);
	}

	bool ActionAddressProxy::CallAction(shared_ptr<PB::NetWorkPacket> message)
	{
		if (!this->mActionSession->IsActive())
		{
			this->mSendQueue.push(message);
			return this->StartConnect();
		}
		return this->mNetWorkManager->SendMessageByAdress(mActionAddress, message);
	}

	bool ActionAddressProxy::StartConnect()
	{
		if (mActionSession->GetState() == Connect)
		{
			return true;
		}
		return mActionSession->StartConnect(BIND_ACTION_2(ActionAddressProxy::OnConnectBack, this));
	}

	void ActionAddressProxy::OnConnectBack(shared_ptr<TcpClientSession> session, bool hasError)
	{
		if (hasError == false)
		{
			while (!this->mSendQueue.empty())
			{
				shared_ptr<PB::NetWorkPacket> message = this->mSendQueue.front();
				this->CallAction(message);
				this->mSendQueue.pop();
			}
		}
		else
		{
			SayNoDebugError("connect " << this->mActionAddress << " fail");
		}
	}
}
