#pragma once
#include<CommonUtil/JsonUtil.h>
#include<ProtocBuf/ProxyProtoc.pb.h>
#include<CommonManager/NetWorkManager.h>
#include<CommonProtocol/ShareSrvClient.pb.h>
namespace SayNo
{
	class TcpClientSession;
	class ProxySrvServerManager : public NetWorkManager
	{
	public:
		ProxySrvServerManager() {}
		~ProxySrvServerManager(){}
	protected:
		void OnSecondUpdate() override;
		void OnFrameUpdate(float t) override;
	};
	SAYNO_OBJECT_TYPE_OF(ProxySrvServerManager);
}