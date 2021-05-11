#include "LoginManager.h"

namespace SoEasy
{
	bool LoginManager::OnInit()
	{
		return true;
	}

	XCode LoginManager::Register(shared_ptr<TcpClientSession>, long long operId, shared_ptr<PB::PlayerRegisterData> registerData, shared_ptr<PB::PlayerRegisterBack> backData)
	{
		return XCode();
	}
}