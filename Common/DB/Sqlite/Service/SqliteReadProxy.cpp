//
// Created by 64658 on 2025/6/6.
//

#include "SqliteReadProxy.h"
#include "Util/Tools/String.h"
#include "DB/Common/SqlFactory.h"
#include "Sqlite/Component/SqliteComponent.h"

namespace acs
{
	SqliteReadProxy::SqliteReadProxy()
	{
		this->mSqlite = nullptr;
	}

	bool SqliteReadProxy::OnInit()
	{
		BIND_RPC_METHOD(SqliteReadProxy::Run)
		BIND_RPC_METHOD(SqliteReadProxy::Func)
		BIND_RPC_METHOD(SqliteReadProxy::Find)
		BIND_RPC_METHOD(SqliteReadProxy::Like)
		BIND_RPC_METHOD(SqliteReadProxy::Count)
		BIND_RPC_METHOD(SqliteReadProxy::FindPage)
		BIND_RPC_METHOD(SqliteReadProxy::Distinct)
		LOG_CHECK_RET_FALSE(this->mSqlite = this->GetComponent<SqliteComponent>())
		return true;
	}

	int SqliteReadProxy::Run(const std::string& sql, json::w::Document& response)
	{
		LOG_ERROR_CHECK_ARGS(!sql.empty())
		std::unique_ptr<sqlite::Response> sqliteResponse = this->mSqlite->Run(sql);
		if(!sqliteResponse->ok)
		{
			response.Add("error", sqliteResponse->error);
			return XCode::Failure;
		}
		std::unique_ptr<json::w::Value> jsonArray = response.AddArray("list");
		for(const std::unique_ptr<json::r::Document> & document : sqliteResponse->result)
		{
			jsonArray->Push(*document);
		}
		response.Add("count", sqliteResponse->count);
		return XCode::Ok;
	}

	int SqliteReadProxy::Func(const json::r::Document& request, rpc::Message& response)
	{
		std::string tab, func, field;
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab));
		LOG_ERROR_CHECK_ARGS(request.Get("func", func));
		LOG_ERROR_CHECK_ARGS(request.Get("field", field));

		json::r::Value filter;
		this->mFactory.GetTable(tab).Func(func, field, field);
		if(request.Get("filter", filter))
		{
			this->mFactory.Filter(filter);
		}
		const std::string sql = this->mFactory.ToString();
		std::unique_ptr<sqlite::Response> sqliteResponse = this->mSqlite->Run(sql);
		LOG_ERROR_RETURN_CODE(sqliteResponse != nullptr, XCode::Failure);

		if(!sqliteResponse->ok)
		{
			response.SetError(sqliteResponse->error);
			return XCode::Failure;
		}
		if(!sqliteResponse->result.empty())
		{
			std::unique_ptr<json::r::Document> & document = sqliteResponse->result.front();
			{
				size_t count = 0;
				std::unique_ptr<char> buffer;
				if(!document->ToCString(buffer, count))
				{
					return XCode::SerializationFailure;
				}
				response.SetContent(rpc::proto::json, buffer.get(), count);
			}
		}
		return XCode::Ok;
	}

	int SqliteReadProxy::Find(const json::r::Document& request, rpc::Message& response)
	{
		std::string tab;
		json::r::Value fields;
		json::r::Value filter;
		request.Get("fields", fields);
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab))
		this->mFactory.GetTable(tab).Select(fields);

		if(request.Get("filter", filter))
		{
			this->mFactory.Filter(filter);
		}
		if(request.Get("sort", filter))
		{
			this->mFactory.OrderBy(filter);
		}
		int limit = 0;
		if(request.Get("limit", limit))
		{
			this->mFactory.Limit(limit);
		}
		const std::string sql = this->mFactory.ToString();
		std::unique_ptr<sqlite::Response> sqliteResponse = this->mSqlite->Run(sql);
		LOG_ERROR_RETURN_CODE(sqliteResponse != nullptr, XCode::Failure);

		if(!sqliteResponse->ok)
		{
			response.SetError(sqliteResponse->error);
			return XCode::Failure;
		}
		json::w::Document jsonArray(true);
		for(const std::unique_ptr<json::r::Document> & document : sqliteResponse->result)
		{
			jsonArray.Push(*document);
		}
		response.SetContent(jsonArray);
		return XCode::Ok;
	}

	int SqliteReadProxy::Like(const json::r::Document& request, rpc::Message& response)
	{
		int limit = 1;
		json::r::Value fields, sort;
		std::string tab, field, value;
		request.Get("limit", limit);
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab))
		LOG_ERROR_CHECK_ARGS(request.Get("field", field))
		LOG_ERROR_CHECK_ARGS(request.Get("value", value))
		LOG_ERROR_CHECK_ARGS(request.Get("fields", fields))
		this->mFactory.GetTable(tab).Select(fields).Like(field, value);
		if(request.Get("sort", sort))
		{
			this->mFactory.OrderBy(sort);
		}
		std::string sql = this->mFactory.Limit(limit).ToString();
		std::unique_ptr<sqlite::Response> sqliteResponse = this->mSqlite->Run(sql);
		LOG_ERROR_RETURN_CODE(sqliteResponse != nullptr, XCode::Failure);

		if(!sqliteResponse->ok)
		{
			response.SetError(sqliteResponse->error);
			return XCode::Failure;
		}
		json::w::Document jsonArray(true);
		for(const std::unique_ptr<json::r::Document> & result : sqliteResponse->result)
		{
			jsonArray.Push(*result);
		}
		response.SetContent(jsonArray);
		return XCode::Ok;
	}

	int SqliteReadProxy::FindOne(const json::r::Document& request, rpc::Message& response)
	{
		std::string tab;
		json::r::Value filter;
		std::vector<std::string> fields;
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab))
		LOG_ERROR_CHECK_ARGS(request.Get("filter", filter))
		{
			this->mFactory.GetTable(tab);
			if(!request.Get("fields", fields))
			{
				this->mFactory.Select();
			}
			else
			{
				this->mFactory.Select(fields);
			}
			this->mFactory.Filter(filter);
		}
		const std::string sql = this->mFactory.Limit(1).ToString();
		std::unique_ptr<sqlite::Response> sqliteResponse = this->mSqlite->Run(sql);
		LOG_ERROR_RETURN_CODE(sqliteResponse != nullptr, XCode::Failure);

		if(!sqliteResponse->ok)
		{
			response.SetError(sqliteResponse->error);
			return XCode::Failure;
		}
		if(!sqliteResponse->result.empty())
		{
			std::unique_ptr<json::r::Document> & document = sqliteResponse->result.front();
			{
				size_t count = 0;
				std::unique_ptr<char> buffer;
				if(!document->ToCString(buffer, count))
				{
					return XCode::SerializationFailure;
				}
				response.SetContent(rpc::proto::json, buffer.get(), count);
			}
		}
		return XCode::Ok;
	}

	int SqliteReadProxy::FindPage(const json::r::Document& request, rpc::Message& response)
	{
		int page = 1;
		int count = 1;
		std::string tab;
		std::vector<std::string> fields;
		{
			request.Get("page", page);
			request.Get("count", count);
			request.Get("fields", fields);
		}
		json::r::Value sort;
		json::r::Value filter;
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab))

		this->mFactory.GetTable(tab).Select(fields);
		if(request.Get("filter", filter))
		{
			this->mFactory.Filter(filter);
		}
		if(request.Get("sort", sort))
		{
			this->mFactory.OrderBy(sort);
		}
		this->mFactory.Page(page, count);
		const std::string sql = this->mFactory.ToString();
		std::unique_ptr<sqlite::Response> sqliteResponse = this->mSqlite->Run(sql);
		LOG_ERROR_RETURN_CODE(sqliteResponse != nullptr, XCode::Failure);

		if(!sqliteResponse->ok)
		{
			response.SetError(sqliteResponse->error);
			return XCode::Failure;
		}
		json::w::Document document(true);
		for(const std::unique_ptr<json::r::Document> & result : sqliteResponse->result)
		{
			document.Push(*result);
		}
		response.SetContent(document);
		return XCode::Ok;
	}

	int SqliteReadProxy::Count(const json::r::Document& request, rpc::Message & response)
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
		std::unique_ptr<sqlite::Response> sqliteResponse = this->mSqlite->Run(sql);
		LOG_ERROR_RETURN_CODE(sqliteResponse != nullptr, XCode::Failure);

		if(!sqliteResponse->ok)
		{
			response.SetError(sqliteResponse->error);
			return XCode::Failure;
		}
		if(!sqliteResponse->result.empty())
		{
			std::unique_ptr<json::r::Document> & document = sqliteResponse->result.front();
			{
				size_t count = 0;
				std::unique_ptr<char> buffer;
				if(!document->ToCString(buffer, count))
				{
					return XCode::SerializationFailure;
				}
				response.SetContent(rpc::proto::json, buffer.get(), count);
			}
		}
		return XCode::Ok;
	}

	int SqliteReadProxy::Distinct(const json::r::Document& request, rpc::Message& response)
	{
		std::string tab;
		json::r::Value filter;
		json::r::Value jsonFields;
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab))
		LOG_ERROR_CHECK_ARGS(request.Get("filter", filter))
		LOG_ERROR_CHECK_ARGS(request.Get("fields", jsonFields))
		LOG_ERROR_CHECK_ARGS(filter.IsObject() && jsonFields.IsArray())
		this->mFactory.GetTable(tab).Distinct(jsonFields).Filter(filter);
		const std::string sql = this->mFactory.ToString();
		std::unique_ptr<sqlite::Response> sqliteResponse = this->mSqlite->Run(sql);
		if(!sqliteResponse->ok)
		{
			response.SetError(sqliteResponse->error);
			return XCode::Failure;
		}
		json::w::Document jsonArray(true);
		for (const std::unique_ptr<json::r::Document>& result: sqliteResponse->result)
		{
			jsonArray.Push(*result);
		}
		response.SetContent(jsonArray);
		return XCode::Ok;
	}
}