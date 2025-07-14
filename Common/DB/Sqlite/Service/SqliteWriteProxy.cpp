//
// Created by 64658 on 2025/6/6.
//

#include "SqliteWriteProxy.h"
#include "Entity/Actor/App.h"
#include "Proto/Component/ProtoComponent.h"
#include "Sqlite/Component/SqliteComponent.h"

namespace acs
{
	SqliteWriteProxy::SqliteWriteProxy()
	{
		this->mSqlite = nullptr;
	}

	bool SqliteWriteProxy::OnInit()
	{
		BIND_RPC_METHOD(SqliteWriteProxy::Run)
		BIND_RPC_METHOD(SqliteWriteProxy::Inc)
		BIND_RPC_METHOD(SqliteWriteProxy::Update)
		BIND_RPC_METHOD(SqliteWriteProxy::Delete)
		BIND_RPC_METHOD(SqliteWriteProxy::Commit)
		BIND_RPC_METHOD(SqliteWriteProxy::Replace)
		BIND_RPC_METHOD(SqliteWriteProxy::SetIndex)
		BIND_RPC_METHOD(SqliteWriteProxy::InsertOne)
		BIND_RPC_METHOD(SqliteWriteProxy::InsertBatch)
		LOG_CHECK_RET_FALSE(this->mSqlite = this->GetComponent<SqliteComponent>())
		return true;
	}

	int SqliteWriteProxy::Run(const std::string & sql, json::w::Document& response)
	{
		LOG_ERROR_CHECK_ARGS(!sql.empty())
		std::unique_ptr<sqlite::Response> sqliteResponse = this->mSqlite->Run(sql);
		if(sqliteResponse == nullptr)
		{
			return XCode::Failure;
		}
		if(sqliteResponse->ok)
		{
			response.Add("error", sqliteResponse->error);
			return XCode::Failure;
		}
		response.Add("count", sqliteResponse->count);
		std::unique_ptr<json::w::Value> jsonArray = response.AddArray("list");
		for(const std::unique_ptr<json::r::Document> & result : sqliteResponse->result)
		{
			jsonArray->Push(*result);
		}
		return XCode::Ok;
	}

	int SqliteWriteProxy::Inc(const json::r::Document& request, json::w::Document& response)
	{
		int value = 1;
		json::r::Value filter;
		std::string tab, field;
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab))
		LOG_ERROR_CHECK_ARGS(request.Get("field", field))
		LOG_ERROR_CHECK_ARGS(request.Get("value", value))
		LOG_ERROR_CHECK_ARGS(request.Get("filter", filter))

		this->mFactory.GetTable(tab).Inc(field, value).Filter(filter);
		const std::string incSql = this->mFactory.Limit(1).ToString();
		this->mFactory.GetTable(tab).Select({ field}).Filter(filter);
		const std::string selectSql = this->mFactory.Limit(1).ToString();
		std::unique_ptr<sqlite::Response> incResponse = this->mSqlite->Run(incSql);
		if(incResponse == nullptr || !incResponse->ok)
		{
			return XCode::Failure;
		}

		std::unique_ptr<sqlite::Response> queryResponse = this->mSqlite->Run(selectSql);
		if(queryResponse == nullptr || !queryResponse->ok)
		{
			return XCode::Failure;
		}

		if(!queryResponse->ok)
		{
			response.Add("error", queryResponse->error);
			return XCode::Failure;
		}
		if(!queryResponse->result.empty())
		{
			std::unique_ptr<json::r::Document> & document = queryResponse->result.front();
			{
				long long number = 0;
				document->Get(field.c_str(), number);
				response.Add(field.c_str(), number);
			}
		}
		return XCode::Ok;
	}

	int SqliteWriteProxy::InsertOne(const json::r::Document& request, json::w::Document& response)
	{
		std::string tab;
		json::r::Value document;
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab))
		LOG_ERROR_CHECK_ARGS(request.Get("document", document))
		this->mFactory.GetTable(tab).Insert(document);
		const std::string sql = this->mFactory.ToString();
		std::unique_ptr<sqlite::Response> sqliteResponse = this->mSqlite->Run(sql);
		if(!sqliteResponse->ok)
		{
			response.Add("error", sqliteResponse->error);
			return XCode::Failure;
		}
		response.Add("count", sqliteResponse->count);
		return XCode::Ok;
	}

	int SqliteWriteProxy::InsertBatch(const json::r::Document& request, json::w::Document& response)
	{
		int count = 0;
		std::string tab;
		json::r::Value documents;
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab))
		LOG_ERROR_CHECK_ARGS(request.Get("documents", documents))
		{
			json::r::Value document;
			this->mSqlite->StartTransaction();
			for (size_t index = 0; index < documents.MemberCount(); index++)
			{
				LOG_ERROR_CHECK_ARGS(documents.Get(index, document))
				LOG_ERROR_CHECK_ARGS(document.GetType() == YYJSON_TYPE_OBJ)
				std::string sql = this->mFactory.GetTable(tab).Insert(document).ToString();
				std::unique_ptr<sqlite::Response> insertResponse = this->mSqlite->Run(sql);
				if(!insertResponse->ok && insertResponse->count <= 0)
				{
					this->mSqlite->RollbackTransaction();
					return XCode::Failure;
				}
				count += insertResponse->count;
			}
			this->mSqlite->CommitTransaction();
		}
		response.Add("count", count);
		return XCode::Ok;
	}

	int SqliteWriteProxy::Replace(const json::r::Document& request, json::w::Document& response)
	{
		std::string tab;
		json::r::Value document;
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab))
		LOG_ERROR_CHECK_ARGS(request.Get("document", document))

		this->mFactory.GetTable(tab).Insert(document);
		const std::string sql = this->mFactory.ToString();
		std::unique_ptr<sqlite::Response> sqliteResponse = this->mSqlite->Run(sql);
		if(!sqliteResponse->ok)
		{
			response.Add("error", sqliteResponse->error);
			return XCode::Failure;
		}
		response.Add("count", sqliteResponse->count);
		return XCode::Ok;
	}

	int SqliteWriteProxy::Delete(const json::r::Document& request, json::w::Document& response)
	{
		std::string tab;
		json::r::Value filter;
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab))
		LOG_ERROR_CHECK_ARGS(request.Get("filter", filter))

		int limit = 0;
		this->mFactory.GetTable(tab).Delete().Filter(filter);
		if(request.Get("limit", limit))
		{
			this->mFactory.Limit(limit);
		}
		const std::string sql = this->mFactory.ToString();
		std::unique_ptr<sqlite::Response> sqliteResponse = this->mSqlite->Run(sql);
		if(!sqliteResponse->ok)
		{
			response.Add("error", sqliteResponse->error);
			return XCode::Failure;
		}
		response.Add("count", sqliteResponse->count);
		return XCode::Ok;
	}

	int SqliteWriteProxy::Update(const json::r::Document& request, json::w::Document& response)
	{
		std::string tab;
		json::r::Value filter;
		json::r::Value document;
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab))
		LOG_ERROR_CHECK_ARGS(request.Get("filter", filter))
		LOG_ERROR_CHECK_ARGS(request.Get("document", document) && document.IsObject())

		int limit = 1;
		this->mFactory.GetTable(tab).Update(document).Filter(filter);
		if(request.Get("limit", limit))
		{
			this->mFactory.Limit(limit);
		}
		const std::string sql = this->mFactory.ToString();
		std::unique_ptr<sqlite::Response> sqliteResponse = this->mSqlite->Run(sql);
		LOG_ERROR_RETURN_CODE(sqliteResponse != nullptr, XCode::Failure);

		if(!sqliteResponse->ok)
		{
			response.Add("error", sqliteResponse->error);
			return XCode::Failure;
		}
		response.Add("count", sqliteResponse->count);
		return XCode::Ok;
	}

	int SqliteWriteProxy::SetIndex(const json::r::Document& request, json::w::Document& response)
	{
		bool unique = false;
		std::string tab, field;
		request.Get("unique", unique);
		LOG_ERROR_CHECK_ARGS(request.Get("tab", tab))
		LOG_ERROR_CHECK_ARGS(request.Get("field", field))

		this->mFactory.GetTable(tab).SetIndex(field, unique);
		const std::string sql = this->mFactory.ToString();
		std::unique_ptr<sqlite::Response> sqliteResponse = this->mSqlite->Run(sql);
		if(!sqliteResponse->ok)
		{
			response.Add("error", sqliteResponse->error);
			return XCode::Failure;
		}
		return XCode::Ok;
	}

	int SqliteWriteProxy::Commit(const json::r::Document& request, json::w::Document& response)
	{
		int okCount = 0;
		size_t count = 0;
		size_t index = 0;
		const char * sql = nullptr;
		std::list<std::unique_ptr<json::r::Document>> content;
		this->mSqlite->StartTransaction();
		{
			do
			{
				sql = request.GetString(index++, count);
				if(sql != nullptr && count > 0)
				{
					std::unique_ptr<sqlite::Response> sqliteResponse = this->mSqlite->Run(sql);
					if(!sqliteResponse->ok)
					{
						this->mSqlite->RollbackTransaction();
						return XCode::Failure;
					}
					okCount += sqliteResponse->count;
					for(std::unique_ptr<json::r::Document> & document : sqliteResponse->result)
					{
						content.emplace_back(std::move(document));
					}
				}
			}
			while(sql != nullptr && count > 0);
			this->mSqlite->CommitTransaction();
		}
		response.Add("count", okCount);
		std::unique_ptr<json::w::Value> jsonArray = response.AddArray("list");
		for(std::unique_ptr<json::r::Document> & document : content)
		{
			jsonArray->Push(*document);
		}
		return XCode::Ok;
	}
}