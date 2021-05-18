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
		std::string address;
		SayNoAssertRetFalse_F(SessionManager::OnInit());
		SayNoAssertRetFalse_F(this->GetConfig().GetValue("ListenAddress", address));
		
		SayNoAssertRetFalse_F(StringHelper::ParseIpAddress(address, this->mConnectIp, this->mConnectPort));
		
		SayNoAssertRetFalse_F(this->mScriptManager = this->GetManager<ScriptManager>());
		SayNoAssertRetFalse_F(this->mCoroutineManager = this->GetManager<CoroutineManager>());
		this->mClientSession = this->CreateTcpSession("Client", this->mConnectIp, this->mConnectPort);
		return this->mClientSession != nullptr;
	}

	void ClientManager::OnInitComplete()
	{
		this->mCoroutineManager->Sleep(1000);
	}

	void ClientManager::OnSessionErrorAfter(shared_ptr<TcpClientSession> tcpSession)
	{
		tcpSession->StartConnect();
	}

	void ClientManager::OnSessionConnectAfter(shared_ptr<TcpClientSession> tcpSession)
	{
		lua_State * luaEnv = this->mScriptManager->GetLuaEnv();
		if (lua_getfunction(luaEnv, "ClientManager", "OnConnectComplate"))
		{
			LuaParameter::Write<SharedTcpSession>(luaEnv, tcpSession);
			if (lua_pcall(luaEnv, 1, 0, 0) != 0)
			{
				SayNoDebugError(lua_tostring(luaEnv, -1));
			}
		}
		/*for (size_t index = 0; index < 100; index++)
		{
			this->mCoroutineManager->Start([this, tcpSession]()
			{
				while (true)
				{
					ActionScheduler shceuder(tcpSession);
					shared_ptr<UserRegisterData> registerData = make_shared<UserRegisterData>();
					registerData->set_phonenum(13716061995);
					registerData->set_platform("iphone_wecaht");
					registerData->set_account("646585122@qq.com");
					registerData->set_password(StringHelper::RandomString(15));

					long long t2 = TimeHelper::GetMilTimestamp();
					XCode code = shceuder.Call("LoginManager.Register", registerData);
					SayNoDebugWarning("register cost time = " << TimeHelper::GetMilTimestamp() - t2 << " code = " << code);

					long long t3 = TimeHelper::GetMilTimestamp();
					shared_ptr<UserAccountData> accountData = make_shared<UserAccountData>();
					accountData->set_account(registerData->account());

					XCode loginCode = shceuder.Call("LoginManager.Login", accountData);
					SayNoDebugWarning("login cost time = " << TimeHelper::GetMilTimestamp() - t3 << " code = " << code);
				}
			});
		}*/
	}
}
