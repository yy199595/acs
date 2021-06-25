#pragma once
#include<Service/ProxyService.h>
namespace SoEasy
{
	class ServiceNode
	{
	public:
		ServiceNode(int areaId, int nodeId, const std::string & address, const std::string & naddress);
	public:
		const int GetAreaId() { return this->mAreaId; }
		const int GetNodeId() { return this->mNodeId; }
	public:
		ProxyService * GetService(const std::string & name);
		ProxyService * CreateProxyService(const int serviceId, const std::string & name);
	public:
		void GetServices(std::vector<ProxyService *> & services);
		const std::string & GetAddress() { return this->mAddress; }
	private:
		const int mAreaId;
		const int mNodeId;
		const std::string mAddress; //监听地址
		const std::string mNoticeAddress; //通信地址
		std::unordered_map<std::string, ProxyService *> mServiceMap;
	};
}