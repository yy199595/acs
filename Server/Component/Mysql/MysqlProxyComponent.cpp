#include "MysqlProxyComponent.h"

#include"Util/StringHelper.h"
#include"Component/Mysql/MysqlService.h"
namespace Sentry
{
	XCode MysqlProxyComponent::Call(const std::string& func, const Message& data, std::shared_ptr<s2s::Mysql::Response> response)
	{
		std::string address;
		LocalServerRpc * mysqlService = this->GetComponent<LocalServerRpc>("MysqlService");
		if(mysqlService == nullptr || !mysqlService->AllotAddress(address))
		{
			return XCode::CallServiceNotFound;
		}
		return mysqlService->Call(address, func, data, response);
	}

	std::shared_ptr<s2s::Mysql::Response> MysqlProxyComponent::Invoke(const std::string& sql)
	{
		s2s::Mysql::Invoke request;
		request.set_sql(sql);
		std::shared_ptr<s2s::Mysql::Response> response(new s2s::Mysql::Response());
		this->Call("Invoke", request, response);
		return response;
	}
}