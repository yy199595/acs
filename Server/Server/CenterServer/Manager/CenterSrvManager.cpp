#include"CenterSrvManager.h"
#include<CommonXCode/XCode.h>
#include<Module/LogicServerDataInfo.h>
#include<CommonCore/Applocation.h>
#include<CommonManager/AddressManager.h>
namespace SoEasy
{
	bool CenterSrvManager::OnInit()
	{
		

		return true;
	}

	void CenterSrvManager::OnInitComplete()
	{
		REGISTER_FUNCTION_1(CenterSrvManager::OnRequestServerList, LogicServerListData);
		REGISTER_FUNCTION_2(CenterSrvManager::OnLogicServerRegister, LogicServerData, LogicServerData);
	}

	XCode CenterSrvManager::OnRequestServerList(shared_ptr<TcpClientSession>, long long id, LogicServerListData & returnData)
	{

		return XCode::Successful;
	}

	XCode CenterSrvManager::OnLogicServerRegister(shared_ptr<TcpClientSession> tcpSession, long long id, const LogicServerData & requestData, LogicServerData & returnData)
	{
		
		return XCode::Successful;
	}
}
