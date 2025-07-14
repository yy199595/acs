//
// Created by 64658 on 2025/6/6.
//

#include "PgsqlWriteProxy.h"
#include "Pgsql/Component/PgsqlDBComponent.h"

namespace acs
{
	PgsqlWriteProxy::PgsqlWriteProxy()
	{
		this->mPgsql = nullptr;
	}

	bool PgsqlWriteProxy::OnInit()
	{
		BIND_RPC_METHOD(PgsqlWriteProxy::Run)
		BIND_RPC_METHOD(PgsqlWriteProxy::Inc)
		BIND_RPC_METHOD(PgsqlWriteProxy::Insert)
		BIND_RPC_METHOD(PgsqlWriteProxy::Update)
		BIND_RPC_METHOD(PgsqlWriteProxy::Delete)
		BIND_RPC_METHOD(PgsqlWriteProxy::Commit)
		BIND_RPC_METHOD(PgsqlWriteProxy::Execute)
		BIND_RPC_METHOD(PgsqlWriteProxy::Replace)
		BIND_RPC_METHOD(PgsqlWriteProxy::InsertBatch)
		BIND_RPC_METHOD(PgsqlWriteProxy::UpdateBatch)
		LOG_CHECK_RET_FALSE(this->mPgsql = this->GetComponent<PgsqlDBComponent>())
		return true;
	}

	int PgsqlWriteProxy::Run(const std::string& sql, json::w::Document& response)
	{
		std::unique_ptr<pgsql::Response> pgsqlResponse = this->mPgsql->Run(sql);
		if (pgsqlResponse == nullptr)
		{
			return XCode::Failure;
		}
		if (!pgsqlResponse->IsOk())
		{
			response.Add("error", pgsqlResponse->error.front());
			return XCode::Failure;
		}
		std::unique_ptr<json::w::Value> jsonArray = response.AddArray("list");
		for (const std::string& result: pgsqlResponse->results)
		{
			jsonArray->AddObject(result.c_str(), result.size());
		}
		return XCode::Ok;
	}

	int PgsqlWriteProxy::Inc(const json::r::Document& request, json::w::Document& response)
	{
		int value = 1;
		json::r::Value filter;
		std::string tab, field;
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab))
		LOG_ERROR_CHECK_ARGS(request.Get("field", field))
		LOG_ERROR_CHECK_ARGS(request.Get("value", value))
		LOG_ERROR_CHECK_ARGS(request.Get("filter", filter))

		this->mFactory.GetTable(tab).Inc(field, value).Filter(filter);

		const std::string incSql = this->mFactory.ToString();
		this->mFactory.GetTable(tab).Select({ field}).Filter(filter);
		const std::string selectSql = this->mFactory.Limit(1).ToString();
		std::unique_ptr<pgsql::Request> pgsqlRequest = std::make_unique<pgsql::Request>();
		{
			pgsqlRequest->AddBatch(incSql);
			pgsqlRequest->AddBatch(selectSql);
		}
		std::unique_ptr<pgsql::Response> pgsqlResponse = this->mPgsql->Run(pgsqlRequest);
		LOG_ERROR_RETURN_CODE(pgsqlResponse != nullptr, XCode::Failure);

		if(!pgsqlResponse->IsOk())
		{
			response.Add("error", pgsqlResponse->error.front());
			return XCode::Failure;
		}
		if(!pgsqlResponse->results.empty())
		{
			json::r::Document document;
			if(document.Decode(pgsqlResponse->results.front()))
			{
				long long number = 0;
				if(document.Get(field.c_str(), number))
				{
					response.Add("value", number);
				}
			}
		}
		return XCode::Ok;
	}

	int PgsqlWriteProxy::Insert(const json::r::Document& request, json::w::Document& response)
	{
		std::string tab;
		json::r::Value document;
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab))
		LOG_ERROR_CHECK_ARGS(request.Get("document", document))

		this->mFactory.GetTable(tab).Insert(document);
		const std::string sql = this->mFactory.ToString();
		std::unique_ptr<pgsql::Response> pgsqlResponse = this->mPgsql->Run(sql);
		if (pgsqlResponse == nullptr)
		{
			return XCode::Failure;
		}
		if (!pgsqlResponse->IsOk())
		{
			response.Add("error", pgsqlResponse->error.front());
			return XCode::Failure;
		}
		response.Add("count", pgsqlResponse->count);
		return XCode::Ok;
	}

	int PgsqlWriteProxy::Replace(const json::r::Document& request, json::w::Document& response)
	{
		std::string tab;
		json::r::Value document;
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab))
		LOG_ERROR_CHECK_ARGS(request.Get("document", document))

		this->mFactory.GetTable(tab).Replace(document);
		const std::string sql = this->mFactory.ToString();
		std::unique_ptr<pgsql::Response> pgsqlResponse = this->mPgsql->Run(sql);
		if (pgsqlResponse == nullptr)
		{
			return XCode::Failure;
		}
		if (!pgsqlResponse->IsOk())
		{
			response.Add("error", pgsqlResponse->error.front());
			return XCode::Failure;
		}
		response.Add("count", pgsqlResponse->count);
		return XCode::Ok;
	}

	int PgsqlWriteProxy::Delete(const json::r::Document& request, json::w::Document& response)
	{
		std::string tab;
		json::r::Value filter;
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab))
		LOG_ERROR_CHECK_ARGS(request.Get("filter", filter))

		this->mFactory.GetTable(tab).Delete().Filter(filter);
		const std::string sql = this->mFactory.ToString();
		std::unique_ptr<pgsql::Response> pgsqlResponse = this->mPgsql->Run(sql);
		if (pgsqlResponse == nullptr || !pgsqlResponse->IsOk())
		{
			return XCode::Failure;
		}
		response.Add("count", pgsqlResponse->okCount);
		return XCode::Ok;
	}

	int PgsqlWriteProxy::Update(const json::r::Document& request, json::w::Document& response)
	{
		std::string tab;
		json::r::Value document, filter;
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab))
		LOG_ERROR_CHECK_ARGS(request.Get("document", document))

		this->mFactory.GetTable(tab).Update(document);
		if (request.Get("filter", filter))
		{
			this->mFactory.Filter(filter);
		}
		const std::string sql = this->mFactory.ToString();
		std::unique_ptr<pgsql::Response> pgsqlResponse = this->mPgsql->Run(sql);
		LOG_ERROR_RETURN_CODE(pgsqlResponse != nullptr, XCode::Failure);

		if (!pgsqlResponse->IsOk())
		{
			response.Add("error", pgsqlResponse->error.front());
			return XCode::Failure;
		}
		response.Add("count", pgsqlResponse->count);
		return XCode::Ok;
	}

	int PgsqlWriteProxy::SetIndex(const json::r::Document& request, json::w::Document& response)
	{
		bool unique = false;
		std::string tab, field;
		request.Get("unique", unique);
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab))
		LOG_ERROR_CHECK_ARGS(request.Get("field", field))
		const std::string sql = this->mFactory.GetTable(tab).SetIndex(field, unique).ToString();
		std::unique_ptr<pgsql::Response> pgsqlResponse = this->mPgsql->Run(sql);
		if (pgsqlResponse == nullptr)
		{
			return XCode::Failure;
		}
		if(!pgsqlResponse->IsOk())
		{
			response.Add("error", pgsqlResponse->error.front());
			return XCode::Failure;
		}
		return XCode::Ok;
	}

	int PgsqlWriteProxy::Execute(const json::r::Document & request, json::w::Document& response)
	{
		std::string stmt;
		std::vector<json::r::Value> jsonArray;
		LOG_ERROR_CHECK_ARGS(request.Get("stmt", stmt))
		LOG_ERROR_CHECK_ARGS(request.Get("args", jsonArray))
		std::string sql = this->mFactory.Execute(stmt, jsonArray).ToString();
		std::unique_ptr<pgsql::Response> pgsqlResponse = this->mPgsql->Run(sql);
		LOG_ERROR_RETURN_CODE(pgsqlResponse != nullptr, XCode::Failure);

		if (!pgsqlResponse->IsOk())
		{
			response.Add("error", pgsqlResponse->error.front());
			return XCode::Failure;
		}
		response.Add("count", pgsqlResponse->count);
		return XCode::Ok;
	}

	int PgsqlWriteProxy::Commit(const json::r::Document& request, json::w::Document& response)
	{
		size_t count = 0;
		size_t index = 0;
		const char * sql = nullptr;
		std::unique_ptr<pgsql::Request> pgsqlRequest = std::make_unique<pgsql::Request>();
		{
			do
			{
				sql = request.GetString(index++, count);
				if(sql != nullptr && count > 0)
				{
					pgsqlRequest->AddBatch(sql, count);
				}
			}
			while(sql != nullptr && count > 0);
		}
		pgsqlRequest->EnableCommit();
		LOG_ERROR_CHECK_ARGS(pgsqlRequest->Count() > 0);
		std::unique_ptr<pgsql::Response> pgsqlResponse = this->mPgsql->Run(pgsqlRequest);
		if(pgsqlResponse == nullptr)
		{
			return XCode::Failure;
		}
		if(!pgsqlResponse->IsOk())
		{
			auto jsonArray = response.AddArray("error");
			for(const std::string & error : pgsqlResponse->error)
			{
				jsonArray->Push(error.c_str(), error.size());
			}
			return XCode::Failure;
		}
		if(!pgsqlResponse->results.empty())
		{
			auto jsonArray = response.AddArray("list");
			for(const std::string & result : pgsqlResponse->results)
			{
				jsonArray->AddObject(result.c_str(), result.size());
			}
		}
		response.Add("count", pgsqlResponse->okCount);
		return XCode::Ok;
	}

	int PgsqlWriteProxy::InsertBatch(const json::r::Document& request, json::w::Document& response)
	{
		std::string tab;
		json::r::Value documents;
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab))
		LOG_ERROR_CHECK_ARGS(request.Get("documents", documents))
		std::unique_ptr<pgsql::Request> pgsqlRequest = std::make_unique<pgsql::Request>();
		{
			json::r::Value document;
			for (size_t index = 0; index < documents.MemberCount(); index++)
			{
				LOG_ERROR_CHECK_ARGS(documents.Get(index, document))
				LOG_ERROR_CHECK_ARGS(document.GetType() == YYJSON_TYPE_OBJ)
				pgsqlRequest->AddBatch(this->mFactory.GetTable(tab).Insert(document).ToString());
			}
		}
		pgsqlRequest->EnableCommit();
		std::unique_ptr<pgsql::Response> pgsqlResponse = this->mPgsql->Run(pgsqlRequest);
		{
			if(pgsqlResponse == nullptr)
			{
				return XCode::Failure;
			}
			if(!pgsqlResponse->IsOk())
			{
				response.Add("count", 0);
				return XCode::Failure;
			}
			response.Add("count", pgsqlResponse->okCount);
		}
		return XCode::Ok;
	}

	int PgsqlWriteProxy::UpdateBatch(const json::r::Document& request, json::w::Document& response)
	{
		std::unique_ptr<pgsql::Request> pgsqlRequest = std::make_unique<pgsql::Request>();
		{
			size_t index = 0;
			json::r::Value jsonValue;
			while(request.Get(index, jsonValue))
			{
				index++;
				std::string table;
				json::r::Value filter;
				json::r::Value document;
				LOG_ERROR_CHECK_ARGS(jsonValue.Get("tab", table))
				LOG_ERROR_CHECK_ARGS(jsonValue.Get("filter", filter))
				LOG_ERROR_CHECK_ARGS(jsonValue.Get("document", document))
				pgsqlRequest->AddBatch(this->mFactory.GetTable(table).Update(document).Filter(filter).ToString());
			}
			pgsqlRequest->IsEnableCommit();
			LOG_ERROR_CHECK_ARGS(pgsqlRequest->Count() > 0);
		}
		std::unique_ptr<pgsql::Response> pgsqlResponse = this->mPgsql->Run(pgsqlRequest);
		{
			if(pgsqlResponse == nullptr)
			{
				return XCode::Failure;
			}
			if(!pgsqlResponse->IsOk())
			{
				std::unique_ptr<json::w::Value> errorArray = response.AddArray("error");
				for(const std::string & error : pgsqlResponse->error)
				{
					errorArray->Push(error);
				}
				return XCode::Failure;
			}
			response.Add("count", pgsqlResponse->okCount);
		}
		return XCode::Ok;
	}
}