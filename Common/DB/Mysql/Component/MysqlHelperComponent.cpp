#ifdef __ENABLE_MYSQL__

#include"MysqlHelperComponent.h"
#include"Util/String/String.h"
#include"Mysql/Service/MysqlDB.h"
#include"Cluster/Config/ClusterConfig.h"
#include"Entity/Actor/App.h"

namespace joke
{
	bool MysqlHelperComponent::Awake()
	{
		const std::string& name = ComponentFactory::GetName<MysqlDB>();
		return ClusterConfig::Inst()->GetServerName(name, this->mServer);
	}

	bool MysqlHelperComponent::LateAwake()
	{
		this->mActComponent = this->GetComponent<ActorComponent>();
		return this->mActComponent != nullptr;
	}

	int MysqlHelperComponent::Add(const Message& message, int flag)
	{
		db::mysql::add request;
		{
			request.set_flag(flag);
			if(!pb_json::MessageToJsonString(message, request.mutable_data()).ok())
			{
				return XCode::ProtoCastJsonFailure;
			}
			request.set_table(message.GetTypeName());
		}
		const static std::string func("MysqlDB.Add");
		Server * targetServer = this->mActComponent->Random(this->mServer);
		if(targetServer == nullptr)
		{
			return XCode::AddressAllotFailure;
		}
		return targetServer->Call(func, request);
	}

	int MysqlHelperComponent::Save(const Message& data, int flag)
	{
		db::mysql::save request;
		{
			request.set_flag(flag);
			request.set_table(data.GetTypeName());
			if(!pb_json::MessageToJsonString(data, request.mutable_data()).ok())
			{
				return XCode::ProtoCastJsonFailure;
			}
		}
		const static std::string func("MysqlDB.Save");
		Server * targetServer = this->mActComponent->Random(this->mServer);
		if(targetServer == nullptr)
		{
			return XCode::AddressAllotFailure;
		}
		return targetServer->Call(func, request);
	}

	int MysqlHelperComponent::Delete(const std::string& table, const std::string& deleteJson, int flag)
	{
		db::mysql::remove request;
		{
			request.set_table(table);
			request.set_where_json(deleteJson);
		}
		const static std::string func("MysqlDB.Delete");
		Server * targetServer = this->mActComponent->Random(this->mServer);
		if(targetServer == nullptr)
		{
			return XCode::AddressAllotFailure;
		}
		return targetServer->Call(func, request);
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
		const static std::string func("MysqlDB.Update");
		Server * targetServer = this->mActComponent->Random(this->mServer);
		if(targetServer == nullptr)
		{
			return XCode::AddressAllotFailure;
		}
		return targetServer->Call(func, request);
	}

	int MysqlHelperComponent::QueryOnce(const std::string& json, pb::Message * response)
	{
		db::mysql::query request;
		{
			request.set_where_json(json);
			request.set_table(response->GetTypeName());
		}
		const static std::string func("MysqlDB.Query");
		Server * targetServer = this->mActComponent->Random(this->mServer);
		if(targetServer == nullptr)
		{
			return XCode::AddressAllotFailure;
		}
		std::unique_ptr<db::mysql::response> result
			= std::unique_ptr<db::mysql::response>();
		int code = targetServer->Call(func, request, result.get());
		if (code != XCode::Ok)
		{
			return code;
		}

		if (result != nullptr && result->jsons_size() > 0)
		{
			const std::string& data = result->jsons(0);
			if (pb_json::JsonStringToMessage(data, response).ok())
			{
				return XCode::Ok;
			}
			return XCode::ParseMessageError;
		}
		return XCode::Ok;
	}
}

#endif