//
// Created by 64658 on 2025/5/12.
//

#include "MongoProxyComponent.h"
#include "Node/Component/NodeComponent.h"
#include "Mongo/Service/MongoReadProxy.h"
#include "Mongo/Service/MongoWriteProxy.h"
#include "Cluster/Config/ClusterConfig.h"

namespace mongo_cmd
{
	constexpr char* MONGODB_INC = "MongoWriteProxy.Inc";
	constexpr char* MONGODB_INSERT = "MongoWriteProxy.Insert";
	constexpr char* MONGODB_UPDATE = "MongoWriteProxy.Update";
	constexpr char* MONGODB_DELETE = "MongoWriteProxy.Delete";
	constexpr char* MONGODB_SET_INDEX = "MongoWriteProxy.SetIndex";

	constexpr char* MONGODB_COUNT = "MongoWriteProxy.Count";
	constexpr char* MONGODB_FIND_ONE = "MongoWriteProxy.FindOne";

}

namespace acs
{
	MongoProxyComponent::MongoProxyComponent()
	{
		this->mNodeMgr = nullptr;
	}

	bool MongoProxyComponent::LateAwake()
	{
		std::string name1 = ComponentFactory::GetName<MongoReadProxy>();
		std::string name2 = ComponentFactory::GetName<MongoWriteProxy>();
		LOG_CHECK_RET_FALSE(this->mNodeMgr = this->GetComponent<NodeComponent>())
		LOG_CHECK_RET_FALSE(ClusterConfig::Inst()->GetServerName(name1, this->mReadName))
		LOG_CHECK_RET_FALSE(ClusterConfig::Inst()->GetServerName(name2, this->mWriteName))
		return true;
	}

	int MongoProxyComponent::Inc(const char* key, int value)
	{
		return this->Inc(nullptr, key, value);
	}

	int MongoProxyComponent::Inc(const char* tab, const char* key, int value)
	{
		int targetValue = 0;
		do
		{
			Node* mongoNode = this->mNodeMgr->Next(this->mWriteName);
			if (mongoNode == nullptr)
			{
				LOG_ERROR("not find service:{}", this->mWriteName);
				break;
			}
			json::w::Document request;
			if (tab != nullptr)
			{
				request.Add("tab", tab);
			}
			request.Add("key", key);
			request.Add("value", value);
			std::unique_ptr<json::r::Document> response = std::make_unique<json::r::Document>();
			if (mongoNode->Call(mongo_cmd::MONGODB_INC, request, response) != XCode::Ok)
			{
				break;
			}
			response->Get("value", targetValue);
		} while (false);
		return targetValue;
	}

	int MongoProxyComponent::UpdateOne(const char* tab, const json::w::Value& filter, const json::w::Value& updater)
	{
		Node* mongoNode = this->mNodeMgr->Next(this->mWriteName);
		if (mongoNode == nullptr)
		{
			LOG_ERROR("not find service:{}", this->mWriteName);
			return XCode::NotFoundActor;
		}
		json::w::Document request;
		{
			request.Add("tab", tab);
			request.Add("limit", 1);
			request.Add("filter", filter);
			request.Add("document", updater);
		}
		return mongoNode->Call(mongo_cmd::MONGODB_UPDATE, request);
	}

	int MongoProxyComponent::DeleteOne(const char* tab, const json::w::Value& filter)
	{
		Node* mongoNode = this->mNodeMgr->Next(this->mWriteName);
		if (mongoNode == nullptr)
		{
			LOG_ERROR("not find service:{}", this->mWriteName);
			return XCode::NotFoundActor;
		}
		json::w::Document request;
		{
			request.Add("tab", tab);
			request.Add("limit", 1);
			request.Add("filter", filter);
		}
		return mongoNode->Call(mongo_cmd::MONGODB_DELETE, request);
	}

	int MongoProxyComponent::Deletes(const char* tab, const json::w::Value& filter, int limit)
	{
		Node* mongoNode = this->mNodeMgr->Next(this->mWriteName);
		if (mongoNode == nullptr)
		{
			LOG_ERROR("not find service:{}", this->mWriteName);
			return XCode::NotFoundActor;
		}
		json::w::Document request;
		{
			request.Add("tab", tab);
			request.Add("limit", limit);
			request.Add("filter", filter);
		}
		return mongoNode->Call(mongo_cmd::MONGODB_DELETE, request);
	}

	int MongoProxyComponent::Insert(const char* tab, const json::w::Value& document)
	{
		Node* mongoNode = this->mNodeMgr->Next(this->mWriteName);
		if (mongoNode == nullptr)
		{
			return XCode::NotFoundActor;
		}
		json::w::Document request;
		{
			request.Add("tab", tab);
			request.AddArray("list")->Push(document);
		}
		return mongoNode->Call(mongo_cmd::MONGODB_INSERT, request);
	}

	int MongoProxyComponent::Insert(const char* tab, const pb::Message& document)
	{
		Node* mongoNode = this->mNodeMgr->Next(this->mWriteName);
		if (mongoNode == nullptr)
		{
			return XCode::NotFoundActor;
		}
		std::string result;
		if (!pb_json::MessageToJsonString(document, &result).ok())
		{
			return XCode::ProtoCastJsonFailure;
		}
		json::w::Document request;
		{
			request.Add("tab", tab);
			request.AddArray("list")->PushObject(result);
		}
		return mongoNode->Call(mongo_cmd::MONGODB_INSERT, request);
	}

	int MongoProxyComponent::SetIndex(const char* tab, const std::string& field, int sort, bool unique)
	{
		Node* mongoNode = this->mNodeMgr->Next(this->mWriteName);
		if (mongoNode == nullptr)
		{
			return XCode::NotFoundActor;
		}
		json::w::Document request;
		{
			request.Add("tab", tab);
			request.Add("key", field);
			request.Add("sort", sort);
			request.Add("unique", unique);
		}
		return mongoNode->Call(mongo_cmd::MONGODB_SET_INDEX, request);
	}

	int MongoProxyComponent::Count(const char* tab)
	{
		Node* mongoNode = this->mNodeMgr->Next(this->mReadName);
		if (mongoNode == nullptr)
		{
			return -1;
		}
		json::w::Document request;
		{
			request.Add("tab", tab);
		}
		std::unique_ptr<json::r::Document> response = std::make_unique<json::r::Document>();
		if (mongoNode->Call(mongo_cmd::MONGODB_COUNT, request, response) != XCode::Ok)
		{
			return -2;
		}
		return response->GetInt("count", 0);
	}

	int MongoProxyComponent::Count(const char* tab, const json::w::Value& filter)
	{
		Node* mongoNode = this->mNodeMgr->Next(this->mReadName);
		if (mongoNode == nullptr)
		{
			return -1;
		}
		json::w::Document request;
		{
			request.Add("tab", tab);
			request.Add("filter", filter);
		}
		std::unique_ptr<json::r::Document> response = std::make_unique<json::r::Document>();
		if (mongoNode->Call(mongo_cmd::MONGODB_COUNT, request, response) != XCode::Ok)
		{
			return -2;
		}
		return response->GetInt("count", 0);
	}

	int MongoProxyComponent::FindOne(const char* tab, const json::w::Value& filter, json::IObject* document)
	{
		std::unique_ptr<json::r::Document> response
				= std::make_unique<json::r::Document>();
		int code = this->FindOne(tab, filter, response);
		if (code != XCode::Ok)
		{
			return code;
		}
		if (!document->Decode(*response))
		{
			return XCode::ParseJsonFailure;
		}
		return XCode::Ok;
	}

	int MongoProxyComponent::FindOne(const char* tab, const json::w::Value& filter, std::unique_ptr<json::r::Document> & document)
	{
		Node* mongoNode = this->mNodeMgr->Next(this->mReadName);
		if (mongoNode == nullptr)
		{
			return XCode::NotFoundActor;
		}
		json::w::Document request;
		{
			request.Add("tab", tab);
			request.Add("filter", filter);
		}
		return mongoNode->Call(mongo_cmd::MONGODB_FIND_ONE, request, document);
	}

	int MongoProxyComponent::FindOne(const char* tab, const json::w::Value& filter,
			const std::vector<std::string>& fields, std::unique_ptr<json::r::Document> & document)
	{
		Node* mongoNode = this->mNodeMgr->Next(this->mReadName);
		if (mongoNode == nullptr)
		{
			return XCode::NotFoundActor;
		}
		json::w::Document request;
		{
			request.Add("tab", tab);
			request.Add("filter", filter);
			request.Add("fields", fields);
		}
		return mongoNode->Call(mongo_cmd::MONGODB_FIND_ONE, request, document);
	}
}