#include"ClientManager.h"
#include<Util/StringHelper.h>
#include<NetWork/RemoteScheduler.h>
#include<Coroutine/CoroutineManager.h>
namespace Client
{
	bool ClientManager::OnInit()
	{
		if (!SessionManager::OnInit())
		{
			return false;
		}
		std::string address;
		if (!this->GetConfig().GetValue("ListenAddress", address))
		{
			SayNoDebugFatal("not find field 'ListenAddress'");
			return false;
		}
		if (!StringHelper::ParseIpAddress(address, this->mConnectIp, this->mConnectPort))
		{
			SayNoDebugFatal("parse 'ListenAddress' fail");
			return false;
		}
		this->mCoroutineManager = this->GetManager<CoroutineManager>();
		this->mClientSession = this->CreateTcpSession("Client", this->mConnectIp, this->mConnectPort);	
		return this->mClientSession != nullptr;
	}

	void ClientManager::OnSessionErrorAfter(shared_ptr<TcpClientSession> tcpSession)
	{

	}

	void ClientManager::OnSessionConnectAfter(shared_ptr<TcpClientSession> tcpSession)
	{
		this->mCoroutineManager->Start([this, tcpSession]()
		{
			while (true)
			{
				StringArray queryInfo;
				RemoteScheduler shceuder(tcpSession);
				queryInfo.add_data_array()->assign("shouhuzhemen299_db");
				queryInfo.add_data_array()->assign("player_space_risk");
				long long t1 = TimeHelper::GetMilTimestamp();
				shceuder.Call("MysqlManager.QueryTable", &queryInfo, [t1](shared_ptr<TcpClientSession>, XCode code)
				{
					long long t2 = TimeHelper::GetMilTimestamp();
					SayNoDebugWarning("cost time = " << t2 - t1);
				});
				this->mCoroutineManager->Sleep(10000);
			}
		});
	}
}
