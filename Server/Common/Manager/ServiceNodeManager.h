#pragma once

#include<Manager/SessionManager.h>
namespace SoEasy
{
	class ServiceNode;
	class ServiceNodeManager : public SessionManager
	{
	public:
		ServiceNodeManager() { }
		~ServiceNodeManager() { }
	public:
		bool DelServiceNode(int nodeId);
		bool DelServiceNode(const std::string & address);
		bool AddServiceNode(ServiceNode * serviceNode);
		SharedTcpSession GetNodeSession(const int nodeId);
	protected:
		bool OnInit() final;
		void OnSystemUpdate() final;
		void OnSessionErrorAfter(SharedTcpSession tcpSession) final;
		void OnSessionConnectAfter(SharedTcpSession tcpSession)final;
	public:
		ServiceNode * GetServiceNode(const int nodeId);
		ServiceNode * GetServiceNode(const std::string & address);
		ServiceNode * GetNodeByNodeName(const std::string & nodeName);
		ServiceNode * GetNodeByServiceName(const std::string & service);
	private:
		std::set<std::string> mConnectSessionList;
		std::list<ServiceNode *> mServiceNodeArray;
		std::unordered_map<int, ServiceNode *> mServiceNodeMap1;
		std::unordered_map<std::string, ServiceNode *> mServiceNodeMap2;
	};
}