

#include"MysqlComponent.h"
#include"Util/Tools/String.h"
#include"Mysql/Service/MysqlDB.h"
#include"Cluster/Config/ClusterConfig.h"
#include"Entity/Actor/App.h"

namespace acs
{
	MysqlComponent::MysqlComponent()
	{
		this->mActor = nullptr;
	}

	bool MysqlComponent::Awake()
	{
		const std::string& name = ComponentFactory::GetName<MysqlDB>();
		return ClusterConfig::Inst()->GetServerName(name, this->mServer);
	}

	bool MysqlComponent::LateAwake()
	{
		LOG_CHECK_RET_FALSE(this->mActor = this->GetComponent<ActorComponent>())
		return true;
	}

	class Server* MysqlComponent::GetActor()
	{
		actor::Group * group = this->mActor->GetGroup(this->mServer);
		if(group == nullptr)
		{
			if(this->mApp->HasComponent("MysqlDB"))
			{
				return this->mApp;
			}
			return nullptr;
		}
		long long id = 0;
		if(!group->Random(id))
		{
			return nullptr;
		}
		return (Server *)this->mActor->GetActor(id);
	}

	int MysqlComponent::Add(const Message& message, int flag)
	{
		db::sql::insert request;
		{
			request.set_flag(flag);
			if(!pb_json::MessageToJsonString(message, request.mutable_document()).ok())
			{
				return XCode::ProtoCastJsonFailure;
			}
			request.set_table(message.GetTypeName());
		}
		const static std::string func("MysqlDB.Add");
		Server * targetServer = this->GetActor();
		if(targetServer == nullptr)
		{
			return XCode::AddressAllotFailure;
		}
		return targetServer->Call(func, request);
	}

	int MysqlComponent::Save(const Message& data, int flag)
	{
		db::sql::save request;
		{
			request.set_flag(flag);
			request.set_table(data.GetTypeName());
			if(!pb_json::MessageToJsonString(data, request.mutable_data()).ok())
			{
				return XCode::ProtoCastJsonFailure;
			}
		}
		const static std::string func("MysqlDB.Save");
		Server * targetServer = this->GetActor();
		if(targetServer == nullptr)
		{
			return XCode::AddressAllotFailure;
		}
		return targetServer->Call(func, request);
	}

	int MysqlComponent::Delete(const std::string& table, const std::string& deleteJson, int flag)
	{
		db::sql::del request;
		{
			request.set_table(table);
			request.set_filter(deleteJson);
		}
		const static std::string func("MysqlDB.Delete");
		Server * targetServer = this->GetActor();
		if(targetServer == nullptr)
		{
			return XCode::AddressAllotFailure;
		}
		return targetServer->Call(func, request);
	}

	int MysqlComponent::Update(const std::string& table, const std::string& updateJson,
			const std::string& whereJson, int flag)
	{
		db::sql::update request;
		{
			request.set_flag(flag);
			request.set_table(table);
			request.set_filter(whereJson);
			request.set_document(updateJson);
		}
		const static std::string func("MysqlDB.Update");
		Server * targetServer = this->GetActor();
		if(targetServer == nullptr)
		{
			return XCode::AddressAllotFailure;
		}
		return targetServer->Call(func, request);
	}

	int MysqlComponent::QueryOnce(const std::string& json, pb::Message * response)
	{
		db::sql::query::request request;
		{
			request.set_filter(json);
			request.set_table(response->GetTypeName());
		}
		Server * targetServer = this->GetActor();
		const static std::string func("MysqlDB.Query");
		if(targetServer == nullptr)
		{
			return XCode::AddressAllotFailure;
		}
		std::unique_ptr<db::sql::query::response> result
			= std::unique_ptr<db::sql::query::response>();
		int code = targetServer->Call(func, request, result.get());
		if (code != XCode::Ok)
		{
			return code;
		}

		if (result != nullptr && result->documents_size() > 0)
		{
			const std::string& data = result->documents(0);
			if (pb_json::JsonStringToMessage(data, response).ok())
			{
				return XCode::Ok;
			}
			return XCode::ParseMessageError;
		}
		return XCode::Ok;
	}
}
