#include "ListenerManager.h"
#include<CommonUtil/StringHelper.h>
#include<CommonCore/Applocation.h>
#include<CommonManager/NetWorkManager.h>
#include<CommonCore/TcpSessionListener.h>
namespace SoEasy
{
	bool ListenerManager::OnInit()
	{
		if (!SessionManager::OnInit())
		{
			return false;
		}
		std::string listenAddress;
		this->GetConfig().GetValue("WhiteList", this->mWhiteList);
		if (!this->GetConfig().GetValue("ListenAddress", listenAddress))
		{
			SayNoDebugError("not find config field ListenAddress");
			return false;
		}
		if (!StringHelper::ParseIpAddress(listenAddress, this->mListenerIp, this->mListenerPort))
		{
			SayNoDebugError("parse ListenAddress fail");
			return false;
		}
		this->mListenAddress = listenAddress;
		this->mTcpSessionListener = make_shared<TcpSessionListener>(this, this->mListenerPort);
		return this->mTcpSessionListener->InitListener();
	}

	void ListenerManager::OnInitComplete()
	{
		this->mTcpSessionListener->StartAcceptConnect();
		SayNoDebugInfo("start listener port " << this->mListenerPort);
	}

	void ListenerManager::OnSessionErrorAfter(shared_ptr<TcpClientSession> tcpSession)
	{

	}

	void ListenerManager::OnSessionConnectAfter(shared_ptr<TcpClientSession> tcpSession)
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
