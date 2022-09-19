#include "MysqlDataComponent.h"

#include"Util/StringHelper.h"
#include"../Service/MysqlService.h"

namespace Sentry
{
	bool MysqlDataComponent::LateAwake()
	{
		this->mMysqlService = this->GetComponent<MysqlService>();
		return this->mMysqlService != nullptr;
	}
	XCode MysqlDataComponent::Add(const Message& message, long long flag)
	{
		db::mysql::add request;
		request.set_flag(flag);
		request.mutable_data()->PackFrom(message);
		request.set_table(message.GetTypeName());
		return this->Call("Add", request);
	}

	XCode MysqlDataComponent::Save(const Message & data, long long flag)
	{
		db::mysql::save request;
		request.set_flag(flag);
		request.set_table(data.GetTypeName());
		request.mutable_data()->PackFrom(data);
		return  this->Call("Save", request);
	}

    XCode MysqlDataComponent::Delete(const std::string &table, const std::string &deleteJson, long long flag)
    {
        db::mysql::remove request;
        request.set_table(table);
        request.set_where_json(deleteJson);
        return this->Call("Delete", request);
    }

    XCode MysqlDataComponent::Update(const std::string &table, const std::string &updateJson,
                                     const std::string &whereJson, long long flag)
    {
        db::mysql::update request;

        request.set_flag(flag);
        request.set_table(table);
        request.set_where_json(whereJson);
        request.set_update_json(updateJson);
        return this->Call("Update", request);
    }


	XCode MysqlDataComponent::Call(const std::string& func, const Message& data, std::shared_ptr<db::mysql::response> response)
	{
		std::string address;
		if(!this->mMysqlService->GetHost(address))
		{
			return XCode::CallServiceNotFound;
		}
		if(response == nullptr)
		{
			return this->mMysqlService->Call(address, func, data);
		}
		return this->mMysqlService->Call(address, func, data, response);
	}

	XCode MysqlDataComponent::QueryOnce(const std::string& json, std::shared_ptr<Message> response, long long flag)
	{
		db::mysql::query request;
		request.set_flag(flag);
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
            const std::string & json = res->jsons(0);
            if(util::JsonStringToMessage(json, response.get()).ok())
            {
                return XCode::Successful;
            }
			return XCode::ParseMessageError;
		}
		return XCode::Successful;
	}
}