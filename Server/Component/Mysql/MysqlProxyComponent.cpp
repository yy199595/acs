#include "MysqlProxyComponent.h"

#include"Util/StringHelper.h"
#include"Component/Mysql/MysqlService.h"
namespace Sentry
{
	XCode MysqlProxyComponent::Add(const Message& data)
	{
		s2s::Mysql::Add request;
		request.set_table(data.GetTypeName());
		request.mutable_data()->PackFrom(data);
		std::shared_ptr<s2s::Mysql::Response>
			response = std::make_shared<s2s::Mysql::Response>();
		return this->Call("Add", request, response);
	}

	XCode MysqlProxyComponent::Save(const Message & data)
	{
		s2s::Mysql::Save request;
		request.set_table(data.GetTypeName());
		request.mutable_data()->PackFrom(data);
		std::shared_ptr<s2s::Mysql::Response>
			response = std::make_shared<s2s::Mysql::Response>();
		return  this->Call("Save", request, response);
	}


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

	XCode MysqlProxyComponent::QueryOnce(const std::string& json, std::shared_ptr<Message> response)
	{
		s2s::Mysql::Query request;
		request.set_where_json(json);
		request.set_table(response->GetTypeName());

		std::shared_ptr<s2s::Mysql::Response>
			res = std::make_shared<s2s::Mysql::Response>();
		XCode code = this->Call("Query", request, res);
		if(code != XCode::Successful)
		{
			return code;
		}

		if (res != nullptr && res->json_array_size() > 0)
		{
			const std::string& json = res->json_array(0);
			if(util::JsonStringToMessage(json, response.get()).ok())
			{
				return XCode::Successful;
			}
			return XCode::JsonCastProtoFailure;
		}
		return XCode::Successful;
	}
}