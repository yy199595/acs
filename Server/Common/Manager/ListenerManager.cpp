#include "ListenerManager.h"
#include<Util/StringHelper.h>
#include<Core/Applocation.h>
#include<Manager/NetWorkManager.h>
#include<Core/TcpSessionListener.h>
namespace SoEasy
{
	bool ListenerManager::OnInit()
	{
		SayNoAssertRetFalse_F(SessionManager::OnInit());
		this->GetConfig().GetValue("WhiteList", this->mWhiteList);
		SayNoAssertRetFalse_F(this->GetConfig().GetValue("ListenAddress", this->mListenAddress));
		SayNoAssertRetFalse_F(StringHelper::ParseIpAddress(this->mListenAddress, this->mListenerIp, this->mListenerPort));

		this->mTcpSessionListener = make_shared<TcpSessionListener>(this, this->mListenerPort);
		SayNoAssertRetFalse_F(this->mTcpSessionListener->InitListener());
		return true;
	}

	void ListenerManager::OnInitComplete()
	{
		this->mTcpSessionListener->StartAcceptConnect();
		SayNoDebugInfo("start listener port " << this->mListenerPort);
	}

	void ListenerManager::OnSessionErrorAfter(SharedTcpSession tcpSession)
	{

	}

	void ListenerManager::OnSessionConnectAfter(SharedTcpSession tcpSession)
	{
		// 判断是否在白名单
		if (!this->mWhiteList.empty())
		{
			const string & ip = tcpSession->GetIP();
			auto iter = this->mWhiteList.find(ip);
			if (iter == this->mWhiteList.end())
			{
				mNetWorkManager->RemoveTcpSession(tcpSession);
				return;
			}
		}
	}

}
