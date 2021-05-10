#include "RemoteActionProxy.h"
#include<Manager/NetWorkManager.h>
#include<Manager/SessionManager.h>
#include<Util/StringHelper.h>
namespace SoEasy
{

	RemoteActionProxy::RemoteActionProxy(NetWorkManager * mgr, const std::string & address)
	{
		this->mNetWorkManager = mgr;
		this->mActionAddress = address;
		SayNoAssertRet_F(this->mSessionManager);
		SayNoAssertRet_F(this->mNetWorkManager);
		SayNoAssertRet_F(StringHelper::ParseIpAddress(address, this->mActionIp, this->mActionPort));
	}

	XCode RemoteActionProxy::CallAction(shared_ptr<PB::NetWorkPacket> message)
	{
		if (!this->mActionSession)
		{
			return XCode::SessionIsNull;
		}
		if (!this->mActionSession->IsActive())
		{
			this->mSendQueue.push(message);
			return XCode::SessionIsNull;
		}
		return this->mNetWorkManager->SendMessageByAdress(mActionAddress, message);
	}

	bool RemoteActionProxy::StartConnect(SessionManager * sMgr)
	{
		if (mActionSession == nullptr)
		{
			mActionSession = make_shared<TcpClientSession>(sMgr, "ActionSession", mActionIp, mActionPort);
		}
		if (mActionSession->GetState() == Connect)
		{
			return true;
		}
		return mActionSession->StartConnect(BIND_ACTION_2(RemoteActionProxy::OnConnectBack, this));
	}

	void RemoteActionProxy::OnConnectBack(shared_ptr<TcpClientSession> session, bool hasError)
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
