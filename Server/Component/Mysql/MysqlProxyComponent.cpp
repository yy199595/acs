#include "MysqlProxyComponent.h"

#include"App/App.h"
#include"Json/JsonWriter.h"
#include"Service/ServiceProxy.h"
#include"Util/StringHelper.h"
#include"Component/Rpc/RpcComponent.h"
#include"Component/Scene/ServiceMgrComponent.h"
#include"Other/ElapsedTimer.h"
#include"DB/Mysql/MysqlRpcTaskSource.h"
namespace Sentry
{
	bool MysqlProxyComponent::Awake()
	{
		this->mCorComponent = nullptr;
		return true;
	}

	bool MysqlProxyComponent::LateAwake()
	{
		this->mCorComponent = App::Get()->GetTaskComponent();
		this->mRpcComponent = this->GetComponent<RpcComponent>();
		this->mServiceComponent = this->GetComponent<ServiceMgrComponent>();
		return true;
	}

	std::shared_ptr<com::Rpc_Request> MysqlProxyComponent::NewMessage(const std::string& name)
	{
		auto mysqlEntity = this->mServiceComponent->GetServiceProxy("MysqlService");
		if (mysqlEntity == nullptr)
		{
			return nullptr;
		}
		std::string func = fmt::format("MysqlService.{0}", name);
		auto requestMessage = mysqlEntity->NewRequest(func);
		if (requestMessage == nullptr)
		{
			LOG_ERROR("not find mysql service method {0}", func);
			return nullptr;
		}
		std::string address = mysqlEntity->AllotAddress();
		if (address.empty())
		{
			LOG_ERROR("allot MysqlService address failure");
			return nullptr;
		}
		std::shared_ptr<ProxyClient> serviceNode = mysqlEntity->GetNode(address);
		if (serviceNode == nullptr)
		{
			LOG_ERROR("not find node : {0}", address);
			return nullptr;
		}
		serviceNode->PushMessage(requestMessage);
		return requestMessage;
	}

	std::shared_ptr<MysqlRpcTaskSource> MysqlProxyComponent::Call(const std::string& func, const Message& data)
	{
		auto requestMessage = this->NewMessage(func);
		if (requestMessage == nullptr)
		{
			return nullptr;
		}
		requestMessage->mutable_data()->PackFrom(data);
		std::shared_ptr<MysqlRpcTaskSource> mysqlRpcTaskSource(new MysqlRpcTaskSource());

		this->mRpcComponent->AddRpcTask(mysqlRpcTaskSource);
		requestMessage->set_rpc_id(mysqlRpcTaskSource->GetRpcId());
#ifdef __DEBUG__
		this->mRpcComponent->AddRpcInfo(mysqlRpcTaskSource->GetRpcId(), requestMessage->method_id());
#endif
		return mysqlRpcTaskSource;
	}

	std::shared_ptr<s2s::Mysql::Response> MysqlProxyComponent::Invoke(const std::string& sql)
	{
		s2s::Mysql::Invoke request;
		request.set_sql(sql);
		auto taskSource = this->Call("Invoke", request);
		return taskSource != nullptr ? taskSource->GetResponse() : nullptr;
	}
}