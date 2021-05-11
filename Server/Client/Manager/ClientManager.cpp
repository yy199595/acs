#include"ClientManager.h"
#include<Util/StringHelper.h>
#include<Protocol/ServerCommon.pb.h>
#include<NetWork/ActionScheduler.h>
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
		tcpSession->StartConnect();
	}

	void ClientManager::OnSessionConnectAfter(shared_ptr<TcpClientSession> tcpSession)
	{
		this->mCoroutineManager->Start([this, tcpSession]()
		{
			this->mCoroutineManager->Sleep(5000);
			long long t1 = TimeHelper::GetMilTimestamp();
			for (size_t index = 0; index < 100; index++)
			{
				ActionScheduler shceuder(tcpSession);
				shared_ptr<PlayerRegisterData> registerData = make_shared<PlayerRegisterData>();
				registerData->set_account("646585122@qq.com");
				registerData->set_password("199595yjz.");

				long long t2 = TimeHelper::GetMilTimestamp();
				XCode code = shceuder.Call("LoginManager.Register", registerData);	
				SayNoDebugWarning("cost time = " << TimeHelper::GetMilTimestamp() - t2);

			}
			SayNoDebugFatal("cost time = " << TimeHelper::GetMilTimestamp() - t1);
		});
	}
}
