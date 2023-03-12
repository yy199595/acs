//
// Created by yjz on 2022/8/28.
//

#include"MongoHelperComponent.h"
#include"Service/MongoDB.h"
#include"App/App.h"
#include"Config/ClusterConfig.h"
#include"Component/NodeMgrComponent.h"
namespace Sentry
{
	bool MongoHelperComponent::LateAwake()
    {      
        this->mMongoDB = this->mApp->GetService<MongoDB>();
        std::string name = ComponentFactory::GetName<MongoDB>();
        this->mLocationComponent = this->GetComponent<NodeMgrComponent>();
        return true;
    }

	int MongoHelperComponent::Insert(const Message& message, int index)
	{
		const std::string tab = message.GetTypeName();
		return this->Insert(tab.c_str(), message, index);
	}

	int MongoHelperComponent::Insert(const char* tab, const Message& message, int index)
	{
		std::string address;
		const std::string & name = this->mMongoDB->GetServer();
		if(!this->mLocationComponent->GetServer(name, address))
		{
			return XCode::AddressAllotFailure;
		}

		db::mongo::insert request;
		request.set_tab(tab);
		request.set_flag(index);
		if(!util::MessageToJsonString(message, request.mutable_json()).ok())
		{
			return XCode::CallServiceNotFound;
		}
		return this->mMongoDB->Call(address, "Insert", request);
	}

	int MongoHelperComponent::Update(const char *tab, const std::string &select, const std::string &data, int index)
    {
		std::string address;
		const std::string & name = this->mMongoDB->GetServer();
		if(!this->mLocationComponent->GetServer(name, address))
		{
			return XCode::AddressAllotFailure;
		}
        db::mongo::update request;
        request.set_tab(tab);
        request.set_update(std::move(data));
        request.set_select(std::move(select));
        return this->mMongoDB->Call(address, "Update", request);
    }

	int MongoHelperComponent::Insert(const char* tab, const std::string& json, int index)
	{
		std::string address;
		const std::string & name = this->mMongoDB->GetServer();
		if(!this->mLocationComponent->GetServer(name, address))
		{
			return XCode::AddressAllotFailure;
		}
		this->mInsertRequest.set_tab(tab);
        this->mInsertRequest.set_flag(index);
        this->mInsertRequest.set_json(std::move(json));
        return this->mMongoDB->Call(address, "Insert", this->mInsertRequest);
	}

	int MongoHelperComponent::Remove(const char* tab, const std::string& select, int limit, int index)
	{
		std::string address;
		const std::string & name = this->mMongoDB->GetServer();
		if (!this->mLocationComponent->GetServer(name, address))
		{
			return XCode::AddressAllotFailure;
		}
		this->mRemoveRequest.set_tab(tab);
        this->mRemoveRequest.set_flag(index);
        this->mRemoveRequest.set_limit(limit);
        this->mRemoveRequest.set_json(std::move(select));
        return this->mMongoDB->Call(address, "Remove", this->mRemoveRequest);
	}

	int MongoHelperComponent::Query(const char* tab,
                                      const std::string& select, std::shared_ptr<Message> response)
	{
		std::string address;
		const std::string & name = this->mMongoDB->GetServer();
		if (!this->mLocationComponent->GetServer(name, address))
		{
			return XCode::AddressAllotFailure;
		}
		this->mQueryRequest.set_tab(tab);
        this->mQueryRequest.set_limit(1);
        this->mQueryRequest.set_json(std::move(select));
		std::shared_ptr<db::mysql::response> result(new db::mysql::response());
		int code = this->mMongoDB->Call(address, "Query", this->mQueryRequest, result);
		if(code == XCode::Successful && result->jsons_size() > 0)
		{
			const std::string & json = result->jsons(0);
			if(!util::JsonStringToMessage(json, response.get()).ok())
			{
				return XCode::JsonCastProtoFailure;
			}
		}
		return code;
	}

	int MongoHelperComponent::Save(const Message &message)
    {
        const Reflection * reflection = message.GetReflection();
        const Descriptor * descriptor = message.GetDescriptor();
        const FieldDescriptor * fileDesc = descriptor->FindFieldByName("_id");
        if(fileDesc == nullptr)
        {
            return XCode::Failure;
        }
        Json::Writer select;
        switch(fileDesc->type())
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
		std::string address;
		const std::string & name = this->mMongoDB->GetServer();
		if (!this->mLocationComponent->GetServer(name, address))
		{
			return XCode::AddressAllotFailure;
		}
        this->mUpdateRequest.Clear();
        if(!util::MessageToJsonString(message, this->mUpdateRequest.mutable_update()).ok())
        {
            return XCode::ProtoCastJsonFailure;
        }
        this->mUpdateRequest.set_tab(std::move(message.GetTypeName()));
        this->mUpdateRequest.set_select(std::move(select.JsonString()));
        return this->mMongoDB->Call(address, "Update", this->mUpdateRequest);
    }

	int MongoHelperComponent::Save(const char *tab, long long id, const std::string &data)
    {
        Json::Writer select;
        select.Add("_id").Add(id);
        return this->Update(tab, select.JsonString(), data, id % 10000);
    }

	int MongoHelperComponent::Save(const char *tab, const std::string &id, const std::string &data)
    {
        Json::Writer select;
        select.Add("_id").Add(id);
        return this->Update(tab, select.JsonString(), data, 0);
    }
}