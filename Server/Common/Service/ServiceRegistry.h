
#pragma once
#include"LocalService.h"
#include<Protocol/com.pb.h>
#include<NetWork/TcpClientSession.h>

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
		XCode RegisterNode(long long id, shared_ptr<s2s::NodeRegister_Request> nodeInfo);
	private:
		int mServiceIndex;
		class NetWorkManager * mNetWorkManager;
		shared_ptr<TcpSessionListener> mTcpSessionListener;
		std::unordered_map<long long, ServiceNode *> mServiceNodeMap;
	};
}