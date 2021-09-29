#include "CenterService.h"
#include <Core/App.h>
#include <Util/StringHelper.h>
#include <Service/ServiceNode.h>
#include <Scene/ProtocolComponent.h>
namespace Sentry
{
    CenterService::CenterService()
    {
    }

	bool CenterService::Awake()
	{
		__add_method(CenterService::Add);
		__add_method(CenterService::Query);
		return LocalServiceComponent::Awake();
	}

    void CenterService::Start()
    {

    }

	XCode CenterService::Add(const s2s::NodeRegister_Request &nodeInfo, s2s::NodeRegister_Response & response)
	{
		const int areaId = nodeInfo.areaid();
		const int nodeId = nodeInfo.nodeid();
		const std::string &address = nodeInfo.address();
		const std::string &nodeName = nodeInfo.servername();
		if (address.empty() || nodeName.empty())
		{
			return XCode::Failure;
		}
		//SharedTcpSession tcpSession = this->GetCurTcpSession();
		const int key = areaId * 10000 + nodeId;
		auto iter = this->mServiceNodeMap.find(key);
		if (iter != this->mServiceNodeMap.end())
		{
			return XCode::Failure;
		}
		ProtocolComponent * protoComponent = this->gameObject->GetComponent<ProtocolComponent>();

		ServiceNode *serviceNode = new ServiceNode(key, nodeName, address);
		for (int index = 0; index < nodeInfo.services_size(); index++)
		{
			const std::string & service = nodeInfo.services(index);
			if (!protoComponent->HasService(service))
			{
				SayNoDebugError("register [ " << service << " ] failure");
				return XCode::Failure;
			}
			serviceNode->AddService(service);
			SayNoDebugLog(nodeName << " add new service [ " << service << " ]");
		}
		response.set_uid(key);
		this->mServiceNodeMap.emplace(key, serviceNode);
		SayNoDebugLog(nodeName << " [" << address << "] register successful ......");

		return XCode::Successful;
	}

	XCode CenterService::Query(const s2s::NodeQuery_Request & service, s2s::NodeQuery_Response & response)
	{
		return XCode::Successful;
	}
}
