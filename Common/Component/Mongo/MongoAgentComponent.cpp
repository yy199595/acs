//
// Created by yjz on 2022/8/28.
//

#include"MongoAgentComponent.h"
#include"MongoService.h"
namespace Sentry
{
	bool MongoAgentComponent::LateAwake()
	{
		this->mMongoService = this->GetComponent<MongoService>();
		return this->mMongoService != nullptr;
	}

	XCode MongoAgentComponent::Insert(const Message& message, int index)
	{
		const std::string tab = message.GetTypeName();
		return this->Insert(tab.c_str(), message, index);
	}

	XCode MongoAgentComponent::Insert(const char* tab, const Message& message, int index)
	{
		std::string address;
		if(!this->mMongoService->GetHost(address))
		{
			return XCode::CallServiceNotFound;
		}
		s2s::mongo::insert request;
		request.set_tab(tab);
		request.set_flag(index);
		if(!util::MessageToJsonString(message, request.mutable_json()).ok())
		{
			return XCode::CallServiceNotFound;
		}
		return this->mMongoService->Call(address, "Insert", request);
	}

	XCode MongoAgentComponent::Insert(const char* tab, const std::string& json, int index)
	{
		std::string address;
		if(!this->mMongoService->GetHost(address))
		{
			return XCode::CallServiceNotFound;
		}
		s2s::mongo::insert request;
		request.set_tab(tab);
		request.set_json(json);
		request.set_flag(index);
		return this->mMongoService->Call(address, "Insert", request);
	}

	XCode MongoAgentComponent::Remove(const char* tab, const std::string& select, int limit, int index)
	{
		std::string address;
		if(!this->mMongoService->GetHost(address))
		{
			return XCode::CallServiceNotFound;
		}
		s2s::mongo::remove request;
		request.set_tab(tab);
		request.set_flag(index);
		request.set_json(select);
		request.set_limit(limit);
		return this->mMongoService->Call(address, "Remove", request);
	}

	XCode MongoAgentComponent::Query(const char* tab,
		const std::string& select, std::shared_ptr<Message> response)
	{
		std::string address;
		if(!this->mMongoService->GetHost(address))
		{
			return XCode::CallServiceNotFound;
		}
		s2s::mongo::query::request request;
		request.set_tab(tab);
		request.set_limit(1);
		request.set_json(select);
		std::shared_ptr<s2s::mongo::query::response> result(new s2s::mongo::query::response());
		XCode code = this->mMongoService->Call(address, "Query", request, result);
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
}