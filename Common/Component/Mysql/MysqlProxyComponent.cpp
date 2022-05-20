#include "MysqlProxyComponent.h"

#include"Util/StringHelper.h"
#include"Component/Mysql/MysqlService.h"
namespace Sentry
{
	bool MysqlProxyComponent::LateAwake()
	{
		this->mMysqlService = this->GetComponent<MysqlService>();
		return this->mMysqlService != nullptr;
	}
	XCode MysqlProxyComponent::Add(const Message& data, long long flage)
	{
		s2s::Mysql::Add request;
		request.set_flag(flage);
		request.mutable_data()->PackFrom(data);
		request.set_table(data.GetTypeName());
		return this->Call("Add", request);
	}

	XCode MysqlProxyComponent::Save(const Message & data, long long flag)
	{
		s2s::Mysql::Save request;
		request.set_flag(flag);
		request.set_table(data.GetTypeName());
		request.mutable_data()->PackFrom(data);
		return  this->Call("Save", request);
	}


	XCode MysqlProxyComponent::Call(const std::string& func, const Message& data, std::shared_ptr<s2s::Mysql::Response> response)
	{
		std::string address;
		if(!this->mMysqlService->AllotAddress(address))
		{
			return XCode::CallServiceNotFound;
		}
		if(response == nullptr)
		{
			return this->mMysqlService->Call(address, func, data);
		}
		return this->mMysqlService->Call(address, func, data, response);
	}

	XCode MysqlProxyComponent::QueryOnce(const std::string& json, std::shared_ptr<Message> response, long long flag)
	{
		s2s::Mysql::Query request;
		request.set_flag(flag);
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