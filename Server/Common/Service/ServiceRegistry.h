
#pragma once
#include"LocalService.h"
#include<NetWork/TcpClientSession.h>
#include<Protocol/ServerCommon.pb.h>
using namespace PB;



namespace SoEasy
{
	class ServiceNode;
	struct ActionProxyInfo
	{
	public:
		int mAreaId;			//服务器组id
		std::string mActionName;	//action名字
		std::string mListenerAddress;	//监听地址
	public:
		bool operator == (ActionProxyInfo & actionInfo)
		{
			return this->mAreaId == actionInfo.mAreaId
				&& this->mActionName == actionInfo.mActionName
				&& this->mListenerAddress == actionInfo.mListenerAddress;
				
		}
	};
	class ProxyService;
	class TcpSessionListener;
	// 所有方法都注册到这里(全局唯一)
	class ServiceRegistry : public LocalService
	{
	public:
		ServiceRegistry();
		~ServiceRegistry() { }
	protected:
		bool OnInit() final;
		void OnSystemUpdate() final;
		void OnInitComplete() final;
	private:
		XCode RegisterNode(long long id, shared_ptr<Service_NodeRegisterRequest> nodeInfo);
		XCode RegisterService(long long id, shared_ptr<Service_RegisterRequest> serviceInfos);
	private:
		XCode RefreshServices(int areaId);
	private:
		int mServiceIndex;
		class NetWorkManager * mNetWorkManager;
		std::vector<ProxyService *> mProxyServices;
		shared_ptr<TcpSessionListener> mTcpSessionListener;
		std::unordered_map<int, ProxyService *> mServiceMap;
		std::unordered_map<long long, ServiceNode *> mServiceNodeMap;
		std::unordered_map<std::string, long long> mQuerySessionMap;
	};
}