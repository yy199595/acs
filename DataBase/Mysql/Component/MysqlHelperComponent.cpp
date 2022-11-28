#include"MysqlHelperComponent.h"
#include"String/StringHelper.h"
#include"Service/MysqlService.h"
#include"Config/ClusterConfig.h"
#include"Component/LocationComponent.h"

namespace Sentry
{
	bool MysqlHelperComponent::LateAwake()
	{
        this->mBindName = ComponentFactory::GetName<MysqlService>();
        LOG_CHECK_RET_FALSE(this->mApp->GetService(this->mBindName));
        this->mLocationComponent = this->GetComponent<LocationComponent>();
		return ClusterConfig::Inst()->GetServerName(this->mBindName, this->mServerName);
	}
	XCode MysqlHelperComponent::Add(const Message& message, int flag)
	{
		db::mysql::add request;
		request.set_flag(flag);
		request.mutable_data()->PackFrom(message);
		request.set_table(message.GetTypeName());
		return this->Call("Add", request);
	}

	XCode MysqlHelperComponent::Save(const Message & data, int flag)
	{
		db::mysql::save request;
		request.set_flag(flag);
		request.set_table(data.GetTypeName());
		request.mutable_data()->PackFrom(data);
		return  this->Call("Save", request);
	}

    XCode MysqlHelperComponent::Delete(const std::string &table, const std::string &deleteJson, int flag)
    {
        db::mysql::remove request;
        request.set_table(table);
        request.set_where_json(deleteJson);
        return this->Call("Delete", request);
    }

    XCode MysqlHelperComponent::Update(const std::string &table, const std::string &updateJson,
                                       const std::string &whereJson, int flag)
    {
        db::mysql::update request;

        request.set_flag(flag);
        request.set_table(table);
        request.set_where_json(whereJson);
        request.set_update_json(updateJson);
        return this->Call("Update", request);
    }


	XCode MysqlHelperComponent::Call(const std::string& func, const Message& data, std::shared_ptr<db::mysql::response> response)
	{
		std::string address;
        RpcService * rpcService = this->mApp->GetService(this->mBindName);
		if(!this->mLocationComponent->AllotServer(this->mServerName, address))
		{
			return XCode::CallServiceNotFound;
		}
		if(response == nullptr)
		{
			return rpcService->Call(address, func, data);
		}
		return rpcService->Call(address, func, data, response);
	}

	XCode MysqlHelperComponent::QueryOnce(const std::string& json, std::shared_ptr<Message> response)
	{
		db::mysql::query request;
		request.set_where_json(json);
		request.set_table(response->GetTypeName());

		std::shared_ptr<db::mysql::response>
			res = std::make_shared<db::mysql::response>();
		XCode code = this->Call("Query", request, res);
		if(code != XCode::Successful)
		{
			return code;
		}

		if (res != nullptr && res->jsons_size() > 0)
		{
            const std::string & data = res->jsons(0);
            if(util::JsonStringToMessage(data, response.get()).ok())
            {
                return XCode::Successful;
            }
			return XCode::ParseMessageError;
		}
		return XCode::Successful;
	}
}