#pragma once
#include<CommonUtil/JsonUtil.h>
#include<CommonProtocol/ShareSrvClient.pb.h>
#include<CommonManager/SayNoClientManager.h>


namespace SayNo
{
	
	class ProxySrvClientManager : public SayNoClientManager
	{
	public:
		ProxySrvClientManager() {}
	protected:

	};
	SAYNO_OBJECT_TYPE_OF(ProxySrvClientManager);
}