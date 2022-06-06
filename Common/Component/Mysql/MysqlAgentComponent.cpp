#include "MysqlAgentComponent.h"

#include"Util/StringHelper.h"
#include"Component/Mysql/MysqlService.h"
#include"Script/Extension/Mysql/LuaMysql.h"

namespace Sentry
{
	bool MysqlAgentComponent::LateAwake()
	{
		this->mMysqlService = this->GetComponent<MysqlService>();
		return this->mMysqlService != nullptr;
	}
	XCode MysqlAgentComponent::Add(const Message& message, long long flage)
	{
		s2s::Mysql::Add request;
		request.set_flag(flage);
		request.mutable_data()->PackFrom(message);
		request.set_table(message.GetTypeName());
		return this->Call("Add", request);
	}

	XCode MysqlAgentComponent::Save(const Message & data, long long flag)
	{
		s2s::Mysql::Save request;
		request.set_flag(flag);
		request.set_table(data.GetTypeName());
		request.mutable_data()->PackFrom(data);
		return  this->Call("Save", request);
	}

    XCode MysqlAgentComponent::Delete(const std::string &table, const std::string &deleteJson, long long flag)
    {
        s2s::Mysql::Delete request;
        request.set_table(table);
        request.set_where_json(deleteJson);
        return this->Call("Delete", request);
    }

    XCode MysqlAgentComponent::Update(const std::string &table, const std::string &updateJson,
                                      const std::string &whereJson, long long flag)
    {
        s2s::Mysql::Update request;

        request.set_flag(flag);
        request.set_table(table);
        request.set_where_json(whereJson);
        request.set_update_json(updateJson);
        return this->Call("Update", request);
    }


	XCode MysqlAgentComponent::Call(const std::string& func, const Message& data, std::shared_ptr<s2s::Mysql::Response> response)
	{
		std::string address;
		if(!this->mMysqlService->GetAddressProxy().GetAddress(address))
		{
			return XCode::CallServiceNotFound;
		}
		if(response == nullptr)
		{
			return this->mMysqlService->Call(address, func, data);
		}
		return this->mMysqlService->Call(address, func, data, response);
	}

	XCode MysqlAgentComponent::QueryOnce(const std::string& json, std::shared_ptr<Message> response, long long flag)
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

    void MysqlAgentComponent::OnLuaRegister(Lua::ClassProxyHelper &luaRegister)
    {
        luaRegister.BeginRegister<MysqlAgentComponent>();
        luaRegister.PushExtensionFunction("Add", Lua::Mysql::Add);
        luaRegister.PushExtensionFunction("Save", Lua::Mysql::Save);
        luaRegister.PushExtensionFunction("Delete", Lua::Mysql::Delete);
        luaRegister.PushExtensionFunction("Update", Lua::Mysql::Update);
        luaRegister.PushExtensionFunction("QueryOnce", Lua::Mysql::QueryOnce);
    }
}