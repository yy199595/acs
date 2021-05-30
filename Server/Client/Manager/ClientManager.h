#pragma once


#include<Manager/ScriptManager.h>
#include<Manager/SessionManager.h>
#include<Protocol/ServerCommon.pb.h>
using namespace SoEasy;
namespace Client
{
	class ClientManager : public SessionManager
	{
	public:
		ClientManager() { }
		~ClientManager() { }
	protected:
		bool OnInit() override;
		void OnInitComplete() override;
		void OnSessionErrorAfter(shared_ptr<TcpClientSession> tcpSession);
		void OnSessionConnectAfter(shared_ptr<TcpClientSession> tcpSession);
	private:
		std::string mConnectIp;
		unsigned short mConnectPort;
		class ScriptManager * mScriptManager;
		CoroutineManager * mCoroutineManager;
		shared_ptr<TcpClientSession> mClientSession;
		
	};
}