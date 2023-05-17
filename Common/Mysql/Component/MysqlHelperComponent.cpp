#ifdef __ENABLE_MYSQL__

#include"MysqlHelperComponent.h"
#include"Util/String/StringHelper.h"
#include"Mysql/Service/MysqlDB.h"
#include"Cluster/Config/ClusterConfig.h"
#include"Rpc/Component/LocationComponent.h"
#include"Entity/Actor/App.h"

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
		db::mysql::add request;
		{
			request.set_flag(flag);
			request.mutable_data()->PackFrom(message);
			request.set_table(message.GetTypeName());
		}
		const std::string func("MysqlDB.Add");
		Actor * targetActor = this->mApp->Random(this->mServer);
		if(targetActor == nullptr)
		{
			return XCode::AddressAllotFailure;
		}
		return targetActor->Call(func, request);
	}

	int MysqlHelperComponent::Save(const Message& data, int flag)
	{
		db::mysql::save request;
		{
			request.set_flag(flag);
			request.set_table(data.GetTypeName());
			request.mutable_data()->PackFrom(data);
		}
		const std::string func("MysqlDB.Save");
		Actor * targetActor = this->mApp->Random(this->mServer);
		if(targetActor == nullptr)
		{
			return XCode::AddressAllotFailure;
		}
		return targetActor->Call(func, request);
	}

	int MysqlHelperComponent::Delete(const std::string& table, const std::string& deleteJson, int flag)
	{
		db::mysql::remove request;
		{
			request.set_table(table);
			request.set_where_json(deleteJson);
		}
		const std::string func("MysqlDB.Delete");
		Actor * targetActor = this->mApp->Random(this->mServer);
		if(targetActor == nullptr)
		{
			return XCode::AddressAllotFailure;
		}
		return targetActor->Call(func, request);
	}

	int MysqlHelperComponent::Update(const std::string& table, const std::string& updateJson,
			const std::string& whereJson, int flag)
	{
		db::mysql::update request;
		{
			request.set_flag(flag);
			request.set_table(table);
			request.set_where_json(whereJson);
			request.set_update_json(updateJson);
		}
		const std::string func("MysqlDB.Update");
		Actor * targetActor = this->mApp->Random(this->mServer);
		if(targetActor == nullptr)
		{
			return XCode::AddressAllotFailure;
		}
		return targetActor->Call(func, request);
	}

	int MysqlHelperComponent::QueryOnce(const std::string& json, std::shared_ptr<Message> response)
	{
		db::mysql::query request;
		{
			request.set_where_json(json);
			request.set_table(response->GetTypeName());
		}

		const std::string func("MysqlDB.Query");
		std::shared_ptr<db::mysql::response>
				result = std::make_shared<db::mysql::response>();
		Actor * targetActor = this->mApp->Random(this->mServer);
		if(targetActor == nullptr)
		{
			return XCode::AddressAllotFailure;
		}
		int code = targetActor->Call(func, request, result);
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