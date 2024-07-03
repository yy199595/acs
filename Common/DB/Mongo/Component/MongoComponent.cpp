//
// Created by yjz on 2022/8/28.
//

#include"MongoComponent.h"
#include"Entity/Actor/App.h"
#include"Mongo/Service/MongoDB.h"
#include"Cluster/Config/ClusterConfig.h"

namespace joke
{
	MongoComponent::MongoComponent()
	{
		this->mActComponent = nullptr;
	}

	bool MongoComponent::Awake()
	{
		const std::string& name = ComponentFactory::GetName<MongoDB>();
		return ClusterConfig::Inst()->GetServerName(name, this->mServer);
	}

	bool MongoComponent::LateAwake()
	{
		LOG_CHECK_RET_FALSE(this->mActComponent = this->GetComponent<ActorComponent>())
		return true;
	}

	int MongoComponent::Insert(const pb::Message& message)
	{
		const std::string tab = message.GetTypeName();
		return this->Insert(tab.c_str(), message);
	}

	int MongoComponent::Insert(const db::mongo::insert& request)
	{
		Server * targetServer = this->GetActor();
		const static std::string func("MongoDB.Insert");
		if(targetServer == nullptr)
		{
			return XCode::AddressAllotFailure;
		}
		return targetServer->Call(func, request);
	}

	int MongoComponent::Inc(const char* key)
	{
		db::mongo::inc::request request;
		{
			request.set_key(key);
		}
		Server * server = this->GetActor();
		if(server == nullptr)
		{
			return -1;
		}
		std::unique_ptr<db::mongo::inc::response>
		        response = std::make_unique<db::mongo::inc::response>();
		int code = server->Call("MongoDB.Inc", request, response.get());
		return code != XCode::Ok ? -2 : response->value();
	}

	int MongoComponent::Inc(const char* tab, json::w::Document& filter, const char* field, int value)
	{
		db::mongo::update request;
		{
			request.set_tab(tab);
			request.set_tag("$inc");
			filter.Encode(request.mutable_document()->mutable_filter());
			{
				json::w::Document updater;
				updater.Add(field, value);
				updater.Encode(request.mutable_document()->mutable_document());
			}
		}
		return this->Update(request);
	}

	int MongoComponent::Insert(const char* tab, const pb::Message& message)
	{
		db::mongo::insert request;
		{
			request.set_tab(tab);
		}
		if (!pb_json::MessageToJsonString(message, request.add_jsons()).ok())
		{
			return XCode::CallServiceNotFound;
		}
		return this->Insert(request);
	}

	int MongoComponent::Update(const char* tab, json::w::Document& select, json::w::Document& update)
	{
		db::mongo::update request;
		{
			request.set_tab(tab);
			request.set_tag("$set");
		}
		select.Encode(request.mutable_document()->mutable_filter());
		update.Encode(request.mutable_document()->mutable_document());
		return this->Update(request);
	}

	int MongoComponent::Update(const db::mongo::update& request)
	{
		const static std::string func("MongoDB.Update");
		Server * targetServer = this->GetActor();
		if(targetServer == nullptr)
		{
			return XCode::AddressAllotFailure;
		}
		return targetServer->Call(func, request);
	}

	int MongoComponent::Update(const db::mongo::updates& request)
	{
		const static std::string func("MongoDB.Updates");
		Server * targetServer = this->GetActor();
		if(targetServer == nullptr)
		{
			return XCode::AddressAllotFailure;
		}
		return targetServer->Call(func, request);
	}

	int MongoComponent::NoWaitUpdate(const db::mongo::update& request)
	{
		const static std::string func("MongoDB.Update");
		Server * targetServer = this->GetActor();
		if(targetServer == nullptr)
		{
			return XCode::AddressAllotFailure;
		}
		return targetServer->Send(func, request);
	}

	class Server* MongoComponent::GetActor()
	{
		if(this->mApp->HasComponent("MongoDB"))
		{
			return this->mApp;
		}
		return this->mActComponent->Random(this->mServer);
	}

	int MongoComponent::Insert(const char* tab, const std::string& json, bool call)
	{
		db::mongo::insert request;
		{
			request.set_tab(tab);
			request.add_jsons(json);
		}
		Server * targetServer = this->GetActor();
		const static std::string func("MongoDB.Insert");
		if(targetServer == nullptr)
		{
			return XCode::AddressAllotFailure;
		}
		if(call) {
			return targetServer->Call(func, request);
		}
		return targetServer->Send(func, request);
	}

	int MongoComponent::Insert(const char* tab, json::w::Document& json, bool call)
	{
		db::mongo::insert request;
		{
			request.set_tab(tab);
		}
		if(!json.Encode(request.add_jsons()))
		{
			return XCode::SerializationFailure;
		}
		Server * targetServer = this->GetActor();
		const static std::string func("MongoDB.Insert");
		if(targetServer == nullptr)
		{
			return XCode::AddressAllotFailure;
		}
		if(call) {
			return targetServer->Call(func, request);
		}
		return targetServer->Send(func, request);
	}

	int MongoComponent::SetIndex(const char* tab, const std::string& field, int sort, bool unique)
	{
		db::mongo::index request;
		{
			request.set_tab(tab);
			request.set_key(field);
			request.set_sort(sort);
			request.set_unique(unique);
		}
		Server * targetServer = this->GetActor();
		const static std::string func("MongoDB.SetIndex");
		if(targetServer == nullptr)
		{
			return XCode::AddressAllotFailure;
		}
		return targetServer->Call(func, request);
	}

	int MongoComponent::Remove(const char* tab, const std::string& select, int limit)
	{
		db::mongo::remove request;
		{
			request.set_tab(tab);
			request.set_limit(limit);
			request.set_json(select);
		}
		Server * targetServer = this->GetActor();
		const static std::string func("MongoDB.Delete");
		if(targetServer == nullptr)
		{
			return XCode::AddressAllotFailure;
		}
		return targetServer->Call(func, request);
	}

	int MongoComponent::Remove(const char* tab, json::w::Document& select, int limit)
	{
		db::mongo::remove request;
		{
			request.set_tab(tab);
			request.set_limit(limit);
			select.Encode(request.mutable_json());
		}
		Server * targetServer = this->GetActor();
		const static std::string func("MongoDB.Delete");
		if(targetServer == nullptr)
		{
			return XCode::AddressAllotFailure;
		}
		return targetServer->Call(func, request);
	}

	std::unique_ptr<db::mongo::find::response> MongoComponent::Find(const char* tab, const std::string& select, int limit)
	{
		db::mongo::find::request request;
		{
			request.set_tab(tab);
			request.set_limit(limit);
			request.set_json(select);
		}
		return this->Find(request);
	}

	std::unique_ptr<db::mongo::find::response> MongoComponent::Find(const db::mongo::find::request& request)
	{
		Server * targetServer = this->GetActor();
		const static std::string func("MongoDB.Find");
		std::unique_ptr<db::mongo::find::response> result
				= std::make_unique<db::mongo::find::response>();
		if(targetServer == nullptr)
		{
			return nullptr;
		}
		int code = targetServer->Call(func, request, result.get());
		return code == XCode::Ok ? std::move(result) : nullptr;
	}

	std::unique_ptr<db::mongo::find::response> MongoComponent::Find(
			const char* tab, json::w::Document& select, int limit)
	{
		db::mongo::find::request request;
		{
			request.set_tab(tab);
			request.set_limit(limit);
		}
		select.Encode(request.mutable_json());
		return this->Find(request);
	}

	int MongoComponent::FindOne(const char* tab, json::w::Document& select, json::r::Document* response)
	{
		db::mongo::find_one::request request;
		{
			request.set_tab(tab);
			select.Encode(request.mutable_where());
		}
		return this->FindOne(request, response);
	}

	int MongoComponent::FindOne(const db::mongo::find_one::request& request, json::r::Document* response)
	{
		std::unique_ptr<db::mongo::find_one::response> result = this->FindOnce(request);
		if(result == nullptr || result->json().empty())
		{
			return XCode::Failure;
		}
		return response->Decode(result->json()) ? XCode::Ok : XCode::ParseJsonFailure;
	}

	std::unique_ptr<db::mongo::find_one::response> MongoComponent::FindOnce(const db::mongo::find_one::request& request)
	{
		Server * targetServer = this->GetActor();
		const static std::string func("MongoDB.FindOne");
		if(targetServer == nullptr)
		{
			return nullptr;
		}
		std::unique_ptr<db::mongo::find_one::response> result
				= std::make_unique<db::mongo::find_one::response>();
		int code = targetServer->Call(func, request, result.get());
		return  code == XCode::Ok ? std::move(result) : nullptr;
	}

	std::unique_ptr<db::mongo::find_modify::response>
	MongoComponent::FindAndModify(const db::mongo::find_modify::request& request)
	{
		Server * targetServer = this->GetActor();
		const static std::string func("MongoDB.FindAndModify");
		if(targetServer == nullptr)
		{
			return nullptr;
		}
		std::unique_ptr<db::mongo::find_modify ::response> result
				= std::make_unique<db::mongo::find_modify::response>();
		int code = targetServer->Call(func, request, result.get());
		return  code == XCode::Ok ? std::move(result) : nullptr;
	}

	int MongoComponent::Query(const char* tab, json::w::Document& filter, json::w::Value* response)
	{
		db::mongo::find::request message;
		{
			message.set_tab(tab);
			filter.Encode(message.mutable_json());
		}
		return this->Query(message, response);
	}

	int MongoComponent::Query(const db::mongo::find::request& request, json::w::Value* response)
	{
		if(!response->IsArray())
		{
			return XCode::CallArgsError;
		}
		std::unique_ptr<db::mongo::find::response> result
				= std::make_unique<db::mongo::find::response>();
		int code = this->Query(request, result.get());
		if(code != XCode::Ok)
		{
			return code;
		}
		if(result->jsons_size() == 0)
		{
			return XCode::NotFoundData;
		}
		for (int i = 0; i < result->jsons_size(); ++i)
		{
			response->PushJson(result->jsons(i));
		}
		return XCode::Ok;
	}

	int MongoComponent::Query(const db::mongo::find::request& request, db::mongo::find::response* response)
	{
		Server * targetServer = this->GetActor();
		const static std::string func("MongoDB.Find");
		if(targetServer == nullptr)
		{
			return XCode::Failure;
		}
		return targetServer->Call(func, request, response);
	}

	int MongoComponent::Count(const char* tab, json::w::Document& doc)
	{
		db::mongo::count::request request;
		{
			request.set_tab(tab);
			doc.Encode(request.mutable_where());
		}
		Server * targetServer = this->GetActor();
		const static std::string func("MongoDB.Count");

		if(targetServer == nullptr)
		{
			return -1;
		}
		std::unique_ptr<db::mongo::count::response>
		        response = std::make_unique<db::mongo::count::response>();
		return targetServer->Call(func, request, response.get()) == XCode::Ok ? response->count() : -1;
	}

	int MongoComponent::FindOne(const char* tab, json::w::Document & document, std::string* response)
	{
		db::mongo::find::request request;
		{
			request.set_tab(tab);
			request.set_limit(1);
			document.Encode(request.mutable_json());
		}
		Server * targetServer = this->GetActor();
		const static std::string func("MongoDB.FindOne");

		if(targetServer == nullptr)
		{
			return XCode::AddressAllotFailure;
		}
		std::unique_ptr<db::mongo::find_one::response> result
				= std::make_unique<db::mongo::find_one::response>();
		int code = targetServer->Call(func, request, result.get());
		if(code == XCode::Ok)
		{
			response->assign(result->json());
		}
		return code;
	}

	int MongoComponent::FindPage(const db::mongo::find_page::request& request, db::mongo::find_page::response* response)
	{
		Server * targetServer = this->GetActor();
		const static std::string func("MongoDB.FindPage");

		if(targetServer == nullptr)
		{
			return XCode::AddressAllotFailure;
		}
		return targetServer->Call(func, request,response);
	}

	int MongoComponent::Query(const char* tab, const std::string& select, pb::Message * response)
	{
		std::unique_ptr<db::mongo::find::response> result = this->Find(tab, select);
		if(result == nullptr)
		{
			return XCode::Failure;
		}
		for(int index = 0; index < result->jsons_size(); index++)
		{
			const std::string& json = result->jsons(index);
			if (!pb_json::JsonStringToMessage(json, response).ok())
			{
				return XCode::JsonCastProtoFailure;
			}
		}
		return XCode::Ok;
	}

	std::unique_ptr<db::mongo::find::response> MongoComponent::Batch(mongo::Batch& batch)
	{
		return this->Find(*batch.Get());
	}
}