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

				const long long num = MathHelper::Random<long long>(10000000, 99999999);
				const long long phonenum = MathHelper::Random<long long>(11111111111, 99999999999);

				registerData->set_phonenum(phonenum);
				registerData->set_platform("iphone_wecaht");
				registerData->set_account(std::to_string(num) + "@qq.com");
				registerData->set_password(StringHelper::RandomString(15));

				long long t2 = TimeHelper::GetMilTimestamp();
				XCode code = shceuder.Call("LoginManager.Register", registerData);	
				SayNoDebugWarning("cost time = " << TimeHelper::GetMilTimestamp() - t2);

			}
			SayNoDebugFatal("cost time = " << TimeHelper::GetMilTimestamp() - t1);
		});
	}
}
