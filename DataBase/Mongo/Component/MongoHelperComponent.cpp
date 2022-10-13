//
// Created by yjz on 2022/8/28.
//

#include"MongoHelperComponent.h"
#include"Service/MongoService.h"
namespace Sentry
{
	bool MongoHelperComponent::LateAwake()
	{
		this->mMongoService = this->GetComponent<MongoService>();
		return this->mMongoService != nullptr;
	}

	XCode MongoHelperComponent::Insert(const Message& message, int index)
	{
		const std::string tab = message.GetTypeName();
		return this->Insert(tab.c_str(), message, index);
	}

	XCode MongoHelperComponent::Insert(const char* tab, const Message& message, int index)
	{
		std::string address;
		if(!this->mMongoService->AllotLocation(address))
		{
			return XCode::CallServiceNotFound;
		}
		db::mongo::insert request;
		request.set_tab(tab);
		request.set_flag(index);
		if(!util::MessageToJsonString(message, request.mutable_json()).ok())
		{
			return XCode::CallServiceNotFound;
		}
		return this->mMongoService->Call(address, "Insert", request);
	}

    XCode MongoHelperComponent::Update(const char *tab, const std::string &select, const std::string &data, int index)
    {
        std::string address;
        if(!this->mMongoService->AllotLocation(address))
        {
            return XCode::CallServiceNotFound;
        }
        db::mongo::update request;
        request.set_tab(tab);
        request.set_update(std::move(data));
        request.set_select(std::move(select));
        return this->mMongoService->Call(address, "Update", request);
    }

	XCode MongoHelperComponent::Insert(const char* tab, const std::string& json, int index)
	{
		std::string address;
		if(!this->mMongoService->AllotLocation(address))
		{
			return XCode::CallServiceNotFound;
		}
		this->mInsertRequest.set_tab(tab);
        this->mInsertRequest.set_flag(index);
        this->mInsertRequest.set_json(std::move(json));
        return this->mMongoService->Call(address, "Insert", this->mInsertRequest);
	}

	XCode MongoHelperComponent::Remove(const char* tab, const std::string& select, int limit, int index)
	{
		std::string address;
		if(!this->mMongoService->AllotLocation(address))
		{
			return XCode::CallServiceNotFound;
		}
		this->mRemoveRequest.set_tab(tab);
        this->mRemoveRequest.set_flag(index);
        this->mRemoveRequest.set_limit(limit);
        this->mRemoveRequest.set_json(std::move(select));
        return this->mMongoService->Call(address, "Remove", this->mRemoveRequest);
	}

	XCode MongoHelperComponent::Query(const char* tab,
                                      const std::string& select, std::shared_ptr<Message> response)
	{
		std::string address;
		if(!this->mMongoService->AllotLocation(address))
		{
			return XCode::CallServiceNotFound;
		}
		this->mQueryRequest.set_tab(tab);
        this->mQueryRequest.set_limit(1);
        this->mQueryRequest.set_json(std::move(select));
		std::shared_ptr<db::mongo::query::response> result(new db::mongo::query::response());
		XCode code = this->mMongoService->Call(address, "Query", this->mQueryRequest, result);
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

    XCode MongoHelperComponent::Save(const Message &message)
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
                select << "_id" << reflection->GetInt32(message, fileDesc);
                break;
            case FieldDescriptor::TYPE_UINT32:
                select << "_id" << reflection->GetUInt32(message, fileDesc);
                break;
            case FieldDescriptor::TYPE_INT64:
                select << "_id" << (long long)reflection->GetInt64(message, fileDesc);
                break;
            case FieldDescriptor::CPPTYPE_UINT64:
                select << "_id" << (unsigned long long)reflection->GetUInt64(message, fileDesc);
                break;
            case FieldDescriptor::TYPE_STRING:
                select << "_id" << reflection->GetString(message, fileDesc);
                break;
            default:
                return XCode::CallArgsError;
        }
        std::string address;
        if(!this->mMongoService->AllotLocation(address))
        {
            return XCode::CallServiceNotFound;
        }
        this->mUpdateRequest.Clear();
        if(!util::MessageToJsonString(message, this->mUpdateRequest.mutable_update()).ok())
        {
            return XCode::ProtoCastJsonFailure;
        }
        this->mUpdateRequest.set_tab(std::move(message.GetTypeName()));
        this->mUpdateRequest.set_select(std::move(select.JsonString()));
        return this->mMongoService->Call(address, "Update", this->mUpdateRequest);
    }

    XCode MongoHelperComponent::Save(const char *tab, long long id, const std::string &data)
    {
        Json::Writer select;
        select << "_id" << id;
        int index = id % 10000;
        return this->Update(tab, select.JsonString(), data, index);
    }

    XCode MongoHelperComponent::Save(const char *tab, const std::string &id, const std::string &data)
    {
        Json::Writer select;
        select << "_id" << id;
        return this->Update(tab, select.JsonString(), data, 0);
    }
}