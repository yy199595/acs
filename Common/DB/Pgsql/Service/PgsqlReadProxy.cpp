//
// Created by 64658 on 2025/6/6.
//

#include "PgsqlReadProxy.h"
#include "Pgsql/Component/PgsqlDBComponent.h"
namespace acs
{
	PgsqlReadProxy::PgsqlReadProxy()
	{
		this->mPgsql = nullptr;
	}

	bool PgsqlReadProxy::OnInit()
	{
		BIND_RPC_METHOD(PgsqlReadProxy::Run)
		BIND_RPC_METHOD(PgsqlReadProxy::Find)
		BIND_RPC_METHOD(PgsqlReadProxy::Func)
		BIND_RPC_METHOD(PgsqlReadProxy::Count)
		BIND_RPC_METHOD(PgsqlReadProxy::Execute)
		BIND_RPC_METHOD(PgsqlReadProxy::FindPage)
		BIND_RPC_METHOD(PgsqlReadProxy::Distinct)
		LOG_CHECK_RET_FALSE(this->mPgsql = this->GetComponent<PgsqlDBComponent>())
		return true;
	}

	int PgsqlReadProxy::Run(const std::string & sql, json::w::Document& response)
	{
		std::unique_ptr<pgsql::Response> pgsqlResponse = this->mPgsql->Run(sql);
		if(pgsqlResponse == nullptr)
		{
			return XCode::Failure;
		}
		if(!pgsqlResponse->IsOk())
		{
			response.Add("error", pgsqlResponse->error.front());
			return XCode::Failure;
		}
		response.Add("count", pgsqlResponse->okCount);
		std::unique_ptr<json::w::Value> jsonArray = response.AddArray("list");
		for(const std::string & result : pgsqlResponse->results)
		{
			jsonArray->AddObject(result.c_str(), result.size());
		}
		return XCode::Ok;
	}

	int PgsqlReadProxy::Find(const json::r::Document& request, rpc::Message& response)
	{
		int limit = 1;
		std::string tab;
		json::r::Value fields;
		request.Get("fields", fields);
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab))

		json::r::Value filter;
		json::r::Value sorter;
		this->mFactory.GetTable(tab).Select(fields);
		if(request.Get("filter", filter))
		{
			this->mFactory.Filter(filter);
		}

		if(request.Get("sort", sorter))
		{
			this->mFactory.OrderBy(sorter);
		}
		if(request.Get("limit", limit))
		{
			this->mFactory.Limit(limit);
		}
		const std::string sql = this->mFactory.ToString();
		std::unique_ptr<pgsql::Response> pgsqlResponse = this->mPgsql->Run(sql);
		LOG_ERROR_RETURN_CODE(pgsqlResponse != nullptr, XCode::Failure);

		if(!pgsqlResponse->IsOk())
		{
			response.SetError(pgsqlResponse->error.front());
			return XCode::Failure;
		}
		json::w::Document jsonArray(true);
		for(const std::string & result : pgsqlResponse->results)
		{
			jsonArray.AddObject(result.c_str(), result.size());
		}
		response.SetContent(jsonArray);
		return XCode::Ok;
	}

	int PgsqlReadProxy::FindOne(const json::r::Document& request, rpc::Message& response)
	{
		std::string tab;
		json::r::Value fields;
		json::r::Value filter;
		request.Get("fields", fields);
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab))
		LOG_ERROR_CHECK_ARGS(request.Get("filter", filter))

		this->mFactory.GetTable(tab).Select(fields).Limit(1);
		const std::string sql = this->mFactory.ToString();
		std::unique_ptr<pgsql::Response> pgsqlResponse = this->mPgsql->Run(sql);
		LOG_ERROR_RETURN_CODE(pgsqlResponse != nullptr, XCode::Failure);

		if(!pgsqlResponse->IsOk())
		{
			response.SetError(pgsqlResponse->error.front());
			return XCode::Failure;
		}
		if(!pgsqlResponse->results.empty())
		{
			response.SetContent(rpc::proto::json, pgsqlResponse->results.front());
		}
		return XCode::Ok;
	}


	int PgsqlReadProxy::FindPage(const json::r::Document& request, rpc::Message& response)
	{
		int page = 1;
		int count = 10;
		std::string tab;
		json::r::Value fields;
		{
			request.Get("page", page);
			request.Get("count", count);
			request.Get("fields", fields);
		}
		json::r::Value sorter;
		json::r::Value filter;
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab))

		this->mFactory.GetTable(tab).Select(fields);

		if(request.Get("filter", filter))
		{
			this->mFactory.Filter(filter);
		}
		if(request.Get("sort", sorter))
		{
			this->mFactory.OrderBy(sorter);
		}
		const std::string sql = this->mFactory.Page(page, count).ToString();
		std::unique_ptr<pgsql::Response> pgsqlResponse = this->mPgsql->Run(sql);
		LOG_ERROR_RETURN_CODE(pgsqlResponse != nullptr, XCode::Failure);

		if(!pgsqlResponse->IsOk())
		{
			response.SetError(pgsqlResponse->error.front());
			return XCode::Failure;
		}
		json::w::Document jsonArray(true);
		for(const std::string & result : pgsqlResponse->results)
		{
			jsonArray.AddObject(result.c_str(), result.size());
		}
		response.SetContent(jsonArray);
		return XCode::Ok;
	}

	int PgsqlReadProxy::Count(const json::r::Document& request, rpc::Message& response)
	{
		std::string tab;
		json::r::Value filter;
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab));
		this->mFactory.GetTable(tab).Count();
		if(request.Get("filter", filter))
		{
			this->mFactory.Filter(filter);
		}
		const std::string sql = this->mFactory.ToString();
		std::unique_ptr<pgsql::Response> pgsqlResponse = this->mPgsql->Run(sql);
		LOG_ERROR_RETURN_CODE(pgsqlResponse != nullptr, XCode::Failure);

		if(!pgsqlResponse->IsOk())
		{
			response.SetError(pgsqlResponse->error.front());
			return XCode::Failure;
		}
		if(!pgsqlResponse->results.empty())
		{
			const std::string & json = pgsqlResponse->results.front();
			response.SetContent(rpc::proto::json, json.c_str(), json.size());
		}
		return XCode::Ok;
	}

	int PgsqlReadProxy::Func(const json::r::Document& request, rpc::Message& response)
	{
		std::string tab, func, field;
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab));
		LOG_ERROR_CHECK_ARGS(request.Get("func", func));
		LOG_ERROR_CHECK_ARGS(request.Get("field", field));

		json::r::Value filter;
		this->mFactory.GetTable(tab);
		if(field == "*")
		{
			this->mFactory.Func(func, field);
		}
		else
		{
			this->mFactory.Func(func, field, field);
		}

		std::string group;
		if(request.Get("group", group))
		{
			this->mFactory.Next().Append(group);
		}
		this->mFactory.From();
		if(request.Get("filter", filter))
		{
			this->mFactory.Filter(filter);
		}
		if(!group.empty())
		{
			this->mFactory.GroupBy(group);
		}

		const std::string sql = this->mFactory.ToString();
		std::unique_ptr<pgsql::Response> pgsqlResponse = this->mPgsql->Run(sql);
		LOG_ERROR_RETURN_CODE(pgsqlResponse != nullptr, XCode::Failure);

		if(!pgsqlResponse->IsOk())
		{
			response.SetError(pgsqlResponse->error.front());
			return XCode::Failure;
		}
		json::w::Document jsonArray(true);
		for(const std::string & json : pgsqlResponse->results)
		{
			jsonArray.PushObject(json);
		}
		response.SetContent(jsonArray);
		return XCode::Ok;
	}

	int PgsqlReadProxy::Execute(const json::r::Document & request, rpc::Message & response)
	{
		std::string stmt;
		std::vector<json::r::Value> argsArray;
		LOG_ERROR_CHECK_ARGS(request.Get("stmt", stmt))
		LOG_ERROR_CHECK_ARGS(request.Get("args", argsArray))
		std::string sql = this->mFactory.Execute(stmt, argsArray).ToString();
		std::unique_ptr<pgsql::Response> pgsqlResponse = this->mPgsql->Run(sql);
		LOG_ERROR_RETURN_CODE(pgsqlResponse != nullptr, XCode::Failure);

		if (!pgsqlResponse->IsOk())
		{
			response.SetError(pgsqlResponse->error.front());
			return XCode::Failure;
		}
		json::w::Document jsonArray(true);
		for(const std::string & result : pgsqlResponse->results)
		{
			jsonArray.AddObject(result.c_str(), result.size());
		}
		response.SetContent(jsonArray);
		return XCode::Ok;
	}

	int PgsqlReadProxy::Distinct(const json::r::Document& request, rpc::Message& response)
	{
		std::string tab;
		json::r::Value filter;
		json::r::Value jsonFields;
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab))
		LOG_ERROR_CHECK_ARGS(request.Get("filter", filter))
		LOG_ERROR_CHECK_ARGS(request.Get("fields", jsonFields))
		LOG_ERROR_CHECK_ARGS(filter.IsObject() && jsonFields.IsArray())
		this->mFactory.GetTable(tab).Distinct(jsonFields).Filter(filter);
		std::unique_ptr<pgsql::Request> mysqlRequest = std::make_unique<pgsql::Request>();
		{
			mysqlRequest->AddBatch(this->mFactory.ToString());
			std::unique_ptr<pgsql::Response> mysqlResponse = this->mPgsql->Run(mysqlRequest);
			LOG_ERROR_RETURN_CODE(mysqlResponse != nullptr, XCode::Failure);

			if(!mysqlResponse->IsOk())
			{
				response.SetError(mysqlResponse->error.front());
				return XCode::Failure;
			}
			json::w::Document jsonArray(true);
			for(const std::string & result : mysqlResponse->results)
			{
				jsonArray.AddObject(result.c_str(), result.size());
			}
			response.SetContent(jsonArray);
		}
		return XCode::Ok;
	}
}