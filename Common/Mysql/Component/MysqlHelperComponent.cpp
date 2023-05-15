#ifdef __ENABLE_MYSQL__

#include"MysqlHelperComponent.h"
#include"Util/String/StringHelper.h"
#include"Mysql/Service/MysqlDB.h"
#include"Cluster/Config/ClusterConfig.h"
#include"Rpc/Component/LocationComponent.h"
#include"Entity/Unit/App.h"

namespace Tendo
{
	bool MysqlHelperComponent::Awake()
	{
		const std::string& name = ComponentFactory::GetName<MysqlDB>();
		return ClusterConfig::Inst()->GetServerName(name, this->mServer);
		return true;
	}

	int MysqlHelperComponent::Add(const Message& message, int flag)
	{
		std::string addr;
		db::mysql::add request;
		{
			request.set_flag(flag);
			request.mutable_data()->PackFrom(message);
			request.set_table(message.GetTypeName());
		}
		if(!this->mApp->GetAddr(this->mServer, addr))
		{
			return XCode::AddressAllotFailure;
		}
		const std::string func("MysqlDB.Add");
		return this->mApp->Call(addr, func, request);
	}

	int MysqlHelperComponent::Save(const Message& data, int flag)
	{
		std::string addr;
		db::mysql::save request;
		{
			request.set_flag(flag);
			request.set_table(data.GetTypeName());
			request.mutable_data()->PackFrom(data);
		}
		if(!this->mApp->GetAddr(this->mServer, addr))
		{
			return XCode::AddressAllotFailure;
		}
		const std::string func("MysqlDB.Save");
		return this->mApp->Call(addr, func, request);
	}

	int MysqlHelperComponent::Delete(const std::string& table, const std::string& deleteJson, int flag)
	{
		std::string addr;
		db::mysql::remove request;
		{
			request.set_table(table);
			request.set_where_json(deleteJson);
		}
		if(!this->mApp->GetAddr(this->mServer, addr))
		{
			return XCode::AddressAllotFailure;
		}
		const std::string func("MysqlDB.Delete");
		return this->mApp->Call(addr, func, request);
	}

	int MysqlHelperComponent::Update(const std::string& table, const std::string& updateJson,
			const std::string& whereJson, int flag)
	{
		std::string addr;
		db::mysql::update request;
		{
			request.set_flag(flag);
			request.set_table(table);
			request.set_where_json(whereJson);
			request.set_update_json(updateJson);
		}
		if(!this->mApp->GetAddr(this->mServer, addr))
		{
			return XCode::AddressAllotFailure;
		}
		const std::string func("MysqlDB.Update");
		return this->mApp->Call(addr, func, request);
	}

	int MysqlHelperComponent::QueryOnce(const std::string& json, std::shared_ptr<Message> response)
	{
		std::string addr;
		db::mysql::query request;
		{
			request.set_where_json(json);
			request.set_table(response->GetTypeName());
		}
		if(!this->mApp->GetAddr(this->mServer, addr))
		{
			return XCode::AddressAllotFailure;
		}

		const std::string func("MysqlDB.Query");
		std::shared_ptr<db::mysql::response>
				result = std::make_shared<db::mysql::response>();
		int code = this->mApp->Call(addr, func, request, result);
		if (code != XCode::Successful)
		{
			return code;
		}

		if (result != nullptr && result->jsons_size() > 0)
		{
			const std::string& data = result->jsons(0);
			if (util::JsonStringToMessage(data, response.get()).ok())
			{
				return XCode::Successful;
			}
			return XCode::ParseMessageError;
		}
		return XCode::Successful;
	}
}

#endif