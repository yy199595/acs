#include"ClientManager.h"
#include<Util/StringHelper.h>
#include<Protocol/ServerCommon.pb.h>
#include<NetWork/ActionScheduler.h>
#include<Coroutine/CoroutineManager.h>
#include<Util/MathHelper.h>
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
			for (size_t index = 0; index < 10; index++)
			{
				ActionScheduler shceuder(tcpSession);
				shared_ptr<UserRegisterData> registerData = make_shared<UserRegisterData>();
				size_t num = 646585122 + index;
				registerData->set_phonenum(13716061995);
				registerData->set_platform("iphone_wecaht");
				registerData->set_account(std::to_string(num) + "@qq.com");
				registerData->set_password(StringHelper::RandomString(15));

				long long t2 = TimeHelper::GetMilTimestamp();
				XCode code = shceuder.Call("LoginManager.Register", registerData);	
				SayNoDebugWarning("cost time = " << TimeHelper::GetMilTimestamp() - t2 << " code = " << code);

			}
			SayNoDebugFatal("cost time = " << TimeHelper::GetMilTimestamp() - t1);
		});
	}
}
