#include"ProxyManager.h"
#include<Util/StringHelper.h>
#include<Util/NumberHelper.h>
#include<Core/TcpSessionListener.h>
#include<Manager/NetWorkManager.h>
namespace SoEasy
{
	void ProxyManager::OnSessionErrorAfter(SharedTcpSession tcpSession)
	{
		
	}

	void ProxyManager::OnSessionConnectAfter(SharedTcpSession tcpSession)
	{	
		
	}

	bool ProxyManager::OnRecvServerMessage(shared_ptr<TcpClientSession> session, shared_ptr<NetWorkPacket> messageData)
	{
		return true;
	}
}