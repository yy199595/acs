#pragma once
#include<CommonManager/AddressManager.h>
#include<CommonProtocol/ServerCommon.pb.h>
using namespace PB;

namespace SoEasy
{
	class CenterSrvManager : public Manager
	{
	public:
		CenterSrvManager() {}
		~CenterSrvManager() {}
	protected:
		bool OnInit() override;
		void OnInitComplete() override;
	private:
		XCode OnRequestServerList(shared_ptr<TcpClientSession>,  long long id, LogicServerListData & returnData);
		XCode OnLogicServerRegister(shared_ptr<TcpClientSession>, long long id, const LogicServerData & requestData, LogicServerData & returnData);
	private:
		SayNoHashMap<int, SayNoArray<GameObject *>> mRegisterSrvMap;
	};
}