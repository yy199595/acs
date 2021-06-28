#pragma once

#include"Manager.h"
namespace SoEasy
{
	class ServiceNode;
	class ServiceNodeManager : public Manager
	{
	public:
		ServiceNodeManager() { }
		~ServiceNodeManager() { }
	public:
		bool DelServiceNode(int nodeId);
		bool DelServiceNode(const std::string & address);
		bool AddServiceNode(ServiceNode * serviceNode);
	protected:
		bool OnInit() final;
		void OnSystemUpdate() final;
	public:
		ServiceNode * GetServiceNode(const int nodeId);
		ServiceNode * GetServiceNode(const std::string & address);
	private:
		std::list<ServiceNode *> mServiceNodeArray;
		std::unordered_map<int, ServiceNode *> mServiceNodeMap1;
		std::unordered_map<std::string, ServiceNode *> mServiceNodeMap2;

	};
}