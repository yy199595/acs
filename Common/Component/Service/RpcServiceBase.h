#pragma once

#include<memory>
#include"Service/ServiceNode.h"
#include"Component/Component.h"


using namespace std;
using namespace com;

namespace Sentry
{
	class ServiceMethod;

	class RpcServiceBase : public Component
	{
	public:
		RpcServiceBase() = default;

		~RpcServiceBase() override = default;

	public:
		virtual void GetServiceName() = 0;
	public:
		virtual std::shared_ptr<ServiceNode> GetNode() = 0;
		virtual void AddNodeAddress(const std::string & address) = 0;
		virtual void DelNodeAddress(const std::string & address) = 0;
		virtual std::shared_ptr<ServiceNode> GetNode(const std::string & address) = 0;
	};

}