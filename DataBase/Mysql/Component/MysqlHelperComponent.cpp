#ifdef __ENABLE_MYSQL__

#include"MysqlHelperComponent.h"
#include"Util/String/StringHelper.h"
#include"Mysql/Service/MysqlDB.h"
#include"Rpc/Component/NodeMgrComponent.h"
#include"Entity/App/App.h"
namespace Sentry
{
	bool MysqlHelperComponent::LateAwake()
	{
        this->mLocationComponent = this->GetComponent<NodeMgrComponent>();
		LOG_CHECK_RET_FALSE(this->mMysqlDB = this->mApp->GetService<MysqlDB>());
		return true;
	}

	int MysqlHelperComponent::Add(const Message& message, int flag)
	{
		db::mysql::add request;
		request.set_flag(flag);
		request.mutable_data()->PackFrom(message);
		request.set_table(message.GetTypeName());
		return this->Call("Add", request);
	}

	int MysqlHelperComponent::Save(const Message & data, int flag)
	{
		db::mysql::save request;
		request.set_flag(flag);
		request.set_table(data.GetTypeName());
		request.mutable_data()->PackFrom(data);
		return  this->Call("Save", request);
	}

    int MysqlHelperComponent::Delete(const std::string &table, const std::string &deleteJson, int flag)
    {
        db::mysql::remove request;
        request.set_table(table);
        request.set_where_json(deleteJson);
        return this->Call("Delete", request);
    }

    int MysqlHelperComponent::Update(const std::string &table, const std::string &updateJson,
                                       const std::string &whereJson, int flag)
    {
        db::mysql::update request;

        request.set_flag(flag);
        request.set_table(table);
        request.set_where_json(whereJson);
        request.set_update_json(updateJson);
        return this->Call("Update", request);
    }

	int MysqlHelperComponent::Call(const std::string& func, const Message& data, std::shared_ptr<db::mysql::response> response)
	{
		std::string address;
		if(!this->mLocationComponent->GetServer(this->mServerName, address))
		{
			return XCode::CallServiceNotFound;
		}
		if(response == nullptr)
		{
			return this->mMysqlDB->Call(address, func, data);
		}
		return this->mMysqlDB->Call(address, func, data, response);
	}

	int MysqlHelperComponent::QueryOnce(const std::string& json, std::shared_ptr<Message> response)
	{
		db::mysql::query request;
		request.set_where_json(json);
		request.set_table(response->GetTypeName());

		std::shared_ptr<db::mysql::response>
			res = std::make_shared<db::mysql::response>();
		int code = this->Call("Query", request, res);
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

#endif