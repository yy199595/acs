//
// Created by yjz on 2022/8/28.
//

#include"MongoHelperComponent.h"
#include"Entity/Actor/App.h"
#include"Mongo/Service/MongoDB.h"
#include"Proto/Include/Message.h"
#include"Cluster/Config/ClusterConfig.h"
namespace Tendo
{
	bool MongoHelperComponent::Awake()
	{
		const std::string& name = ComponentFactory::GetName<MongoDB>();
		return ClusterConfig::Inst()->GetServerName(name, this->mServer);
	}

	int MongoHelperComponent::Insert(const Message& message, int index)
	{
		const std::string tab = message.GetTypeName();
		return this->Insert(tab.c_str(), message, index);
	}

	int MongoHelperComponent::Insert(const char* tab, const Message& message, int index)
	{
		db::mongo::insert request;
		{
			request.set_tab(tab);
			request.set_flag(index);
		}
		const std::string func("MongoDB.Insert");
		ServerActor * targetActor = this->mApp->ActorMgr()->Random(this->mServer);
		if (!pb_json::MessageToJsonString(message, request.mutable_json()).ok())
		{
			return XCode::CallServiceNotFound;
		}
		if(targetActor == nullptr)
		{
			return XCode::AddressAllotFailure;
		}
		return targetActor->Call(func, request);
	}

	int MongoHelperComponent::Update(const char* tab, const std::string& select, const std::string& data, int index)
	{
		db::mongo::update request;
		{
			request.set_tab(tab);
			request.set_update(data);
			request.set_select(select);
		}
		const std::string func("MongoDB.Update");
		ServerActor * targetActor = this->mApp->ActorMgr()->Random(this->mServer);
		if(targetActor == nullptr)
		{
			return XCode::AddressAllotFailure;
		}
		return targetActor->Call(func, request);
	}

	int MongoHelperComponent::Insert(const char* tab, const std::string& json, int index)
	{
		db::mongo::insert request;
		{
			request.set_tab(tab);
			request.set_flag(index);
			request.set_json(json);
		}
		const std::string func("MongoDB.Insert");
		ServerActor * targetActor = this->mApp->ActorMgr()->Random(this->mServer);
		if(targetActor == nullptr)
		{
			return XCode::AddressAllotFailure;
		}
		return targetActor->Call(func, request);
	}

	int MongoHelperComponent::Remove(const char* tab, const std::string& select, int limit, int index)
	{
		db::mongo::remove request;
		{
			request.set_tab(tab);
			request.set_flag(index);
			request.set_limit(limit);
			request.set_json(select);
		}
		const std::string func("MongoDB.Remove");
		ServerActor * targetActor = this->mApp->ActorMgr()->Random(this->mServer);
		if(targetActor == nullptr)
		{
			return XCode::AddressAllotFailure;
		}
		return targetActor->Call(func, request);
	}

	int MongoHelperComponent::Query(const char* tab,
			const std::string& select, std::shared_ptr<Message> response)
	{
		db::mongo::query::request request;
		{
			request.set_tab(tab);
			request.set_limit(1);
			request.set_json(select);
		}
		const std::string func("MongoDB.Query");
		std::shared_ptr<db::mongo::query::response> result
			= std::make_shared<db::mongo::query::response>();
		ServerActor * targetActor = this->mApp->ActorMgr()->Random(this->mServer);
		if(targetActor == nullptr)
		{
			return XCode::AddressAllotFailure;
		}
		int code = targetActor->Call(func, request, result);
		if(code == XCode::Successful && result->jsons_size() > 0)
		{
			const std::string& json = result->jsons(0);
			if (!util::JsonStringToMessage(json, response.get()).ok())
			{
				return XCode::JsonCastProtoFailure;
			}
		}
		return code;
	}

	int MongoHelperComponent::Save(const Message& message)
	{
		const Reflection* reflection = message.GetReflection();
		const Descriptor* descriptor = message.GetDescriptor();
		const FieldDescriptor* fileDesc = descriptor->FindFieldByName("_id");
		if (fileDesc == nullptr)
		{
			return XCode::Failure;
		}
		Json::Writer select;
		switch (fileDesc->type())
		{
			case FieldDescriptor::TYPE_INT32:
				select.Add("_id").Add(reflection->GetInt32(message, fileDesc));
				break;
			case FieldDescriptor::TYPE_UINT32:
				select.Add("_id").Add(reflection->GetUInt32(message, fileDesc));
				break;
			case FieldDescriptor::TYPE_INT64:
				select.Add("_id").Add((long long)reflection->GetInt64(message, fileDesc));
				break;
			case FieldDescriptor::CPPTYPE_UINT64:
				select.Add("_id").Add((unsigned long long)reflection->GetUInt64(message, fileDesc));
				break;
			case FieldDescriptor::TYPE_STRING:
				select.Add("_id").Add(reflection->GetString(message, fileDesc));
				break;
			default:
				return XCode::CallArgsError;
		}

		db::mongo::update request;
		if (!util::MessageToJsonString(message, request.mutable_update()).ok())
		{
			return XCode::ProtoCastJsonFailure;
		}
		request.set_tab(message.GetTypeName());
		request.set_select(select.JsonString());
		const std::string func("MongoDB.Update");
		ServerActor * targetActor = this->mApp->ActorMgr()->Random(this->mServer);
		if(targetActor == nullptr)
		{
			return XCode::AddressAllotFailure;
		}
		return targetActor->Call(func, request);
	}

	int MongoHelperComponent::Save(const char* tab, long long id, const std::string& data)
	{
		Json::Writer select;
		select.Add("_id").Add(id);
		return this->Update(tab, select.JsonString(), data, id % 10000);
	}

	int MongoHelperComponent::Save(const char* tab, const std::string& id, const std::string& data)
	{
		Json::Writer select;
		select.Add("_id").Add(id);
		return this->Update(tab, select.JsonString(), data, 0);
	}
}