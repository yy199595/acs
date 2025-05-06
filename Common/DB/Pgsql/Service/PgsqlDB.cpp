

#include"PgsqlDB.h"
#include"Entity/Actor/App.h"
#include"Proto/Component/ProtoComponent.h"
#include"Pgsql/Component/PgsqlDBComponent.h"

namespace acs
{
	PgsqlDB::PgsqlDB()
	{
		this->mProto = nullptr;
		this->mPgsql = nullptr;
	}

	bool PgsqlDB::Awake()
	{
		this->mApp->AddComponent<PgsqlDBComponent>();
		return true;
	}

	bool PgsqlDB::OnInit()
	{
		BIND_RPC_METHOD(PgsqlDB::Save);
		BIND_RPC_METHOD(PgsqlDB::Exec);
		BIND_RPC_METHOD(PgsqlDB::Query);
		BIND_RPC_METHOD(PgsqlDB::Index);
		BIND_RPC_METHOD(PgsqlDB::Update);
		BIND_RPC_METHOD(PgsqlDB::Delete);
		BIND_RPC_METHOD(PgsqlDB::Create);
		BIND_RPC_METHOD(PgsqlDB::Insert);
		BIND_RPC_METHOD(PgsqlDB::FindPage);
		LOG_CHECK_RET_FALSE(this->mProto = this->GetComponent<ProtoComponent>())
		LOG_CHECK_RET_FALSE(this->mPgsql = this->GetComponent<PgsqlDBComponent>())
		return true;
	}

    void PgsqlDB::OnDestroy()
    {

    }

    int PgsqlDB::Create(const db::sql::create &request)
    {
        std::unique_ptr<Message> message;
		const std::string & table = request.table();
		if(!this->mProto->New(table, message))
		{
			return XCode::CallArgsError;
		}
        std::vector<std::string> keys;
        const Descriptor * descriptor = message->GetDescriptor();
        for (const std::string &key: request.keys())
        {
            const FieldDescriptor * fieldDescriptor = descriptor->FindFieldByName(key);
            if(fieldDescriptor == nullptr)
            {
                return XCode::CallArgsError;
            }
            switch(fieldDescriptor->type())
            {
                case FieldDescriptor::Type::TYPE_INT32:
                case FieldDescriptor::Type::TYPE_INT64:
                case FieldDescriptor::Type::TYPE_UINT32:
                case FieldDescriptor::Type::TYPE_UINT64:
                case FieldDescriptor::Type::TYPE_STRING:
                    break;
                default:
                    return XCode::CallArgsError;
            }
            keys.emplace_back(key);
        }
        if(keys.size() == 1)
        {
            this->mMainKeys[table] = keys[0];
        }

		return XCode::Ok;
	}

	int PgsqlDB::Index(const db::sql::index& request)
	{
		std::string sql;
		if(!this->mSqlHelper.CreateIndex(request,sql))
		{
			return XCode::CallArgsError;
		}
		std::unique_ptr<pgsql::Response> pgsqlResponse = this->mPgsql->Run(sql);
		return pgsqlResponse && pgsqlResponse->IsOk() ? XCode::Ok : XCode::Failure;
	}

	int PgsqlDB::Insert(const db::sql::insert& request, db::sql::response &response)
    {
		std::string sql;
		if(!this->mSqlHelper.Insert(request, sql))
		{
			return XCode::Failure;
		}
		std::unique_ptr<pgsql::Response> pgsqlResponse = this->mPgsql->Run(sql);
		LOG_ERROR_RETURN_CODE(pgsqlResponse != nullptr, XCode::Failure);
		if(!pgsqlResponse->IsOk())
		{
			response.set_error(pgsqlResponse->mError);
			return XCode::Failure;
		}
		response.set_count(pgsqlResponse->count);
		return XCode::Ok;
    }

	int PgsqlDB::Save(const db::sql::save& request, db::sql::response &response)
    {
		std::string sql;
		if (!this->mSqlHelper.Replace(request, sql))
        {
            return XCode::CallArgsError;
        }
		std::unique_ptr<pgsql::Response> pgsqlResponse = this->mPgsql->Run(sql);
		LOG_ERROR_RETURN_CODE(pgsqlResponse != nullptr, XCode::Failure);
		if(!pgsqlResponse->IsOk())
		{
			response.set_error(pgsqlResponse->mError);
			return XCode::Failure;
		}
		response.set_count(pgsqlResponse->count);
		return XCode::Ok;
    }

	int PgsqlDB::Update(const db::sql::update& request, db::sql::response & response)
    {
        std::string sql;
        if (!this->mSqlHelper.Update(request, sql))
        {
            return XCode::CallArgsError;
        }
		std::unique_ptr<pgsql::Response> pgsqlResponse = this->mPgsql->Run(sql);
		LOG_ERROR_RETURN_CODE(pgsqlResponse != nullptr, XCode::Failure);

		if(!pgsqlResponse->IsOk())
		{
			response.set_error(pgsqlResponse->mError);
			return XCode::Failure;
		}
		response.set_count(pgsqlResponse->count);
		return XCode::Ok;
    }

	int PgsqlDB::Delete(const db::sql::del& request, db::sql::response &response)
    {
        std::string sql;
		if (!this->mSqlHelper.Delete(request, sql))
        {
            return XCode::CallArgsError;
        }
		std::unique_ptr<pgsql::Response> pgsqlResponse = this->mPgsql->Run(sql);
		if(pgsqlResponse == nullptr || !pgsqlResponse->IsOk())
		{
			return XCode::Failure;
		}
		return XCode::Ok;
    }

	int PgsqlDB::Exec(const db::sql::exec& request, db::sql::query::response & response)
	{
		const std::string & sql = request.sql();
		std::unique_ptr<pgsql::Response> pgsqlResponse = this->mPgsql->Run(sql);
		LOG_ERROR_RETURN_CODE(pgsqlResponse != nullptr, XCode::Failure);

		if(!pgsqlResponse->IsOk())
		{
			response.set_error(pgsqlResponse->mError);
			return XCode::Failure;
		}
		for(const std::string & result : pgsqlResponse->mResults)
		{
			response.add_documents(result);
		}
		return XCode::Ok;
	}

	int PgsqlDB::Query(const db::sql::query::request& request, db::sql::query::response& response)
	{
		std::string sql;
		if (!this->mSqlHelper.Select(request, sql))
		{
			return XCode::CallArgsError;
		}
		std::unique_ptr<pgsql::Response> pgsqlResponse = this->mPgsql->Run(sql);
		LOG_ERROR_RETURN_CODE(pgsqlResponse != nullptr, XCode::Failure);

		if(!pgsqlResponse->IsOk())
		{
			response.set_error(pgsqlResponse->mError);
			return XCode::Failure;
		}
		for(const std::string & result : pgsqlResponse->mResults)
		{
			response.add_documents(result);
		}
		return XCode::Ok;
	}

	int PgsqlDB::FindPage(const db::sql::query::page& request, db::sql::query::response& response)
	{
		std::string sql;
		if (!this->mSqlHelper.FindPage(request, sql))
		{
			return XCode::CallArgsError;
		}
		int offset = (request.page() - 1) * request.limit();
		sql.append(fmt::format(" LIMIT {}, {}", offset, request.limit()));
		std::unique_ptr<pgsql::Response> pgsqlResponse = this->mPgsql->Run(sql);
		LOG_ERROR_RETURN_CODE(pgsqlResponse != nullptr, XCode::Failure);

		if(!pgsqlResponse->IsOk())
		{
			response.set_error(pgsqlResponse->mError);
			return XCode::Failure;
		}
		for(const std::string & result : pgsqlResponse->mResults)
		{
			response.add_documents(result);
		}
		return XCode::Ok;
	}
}// namespace Sentry
