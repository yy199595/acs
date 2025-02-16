

#include"MysqlDB.h"
#include"Entity/Actor/App.h"
#include"Proto/Component/ProtoComponent.h"
#include"Mysql/Component/MysqlDBComponent.h"

namespace acs
{
	MysqlDB::MysqlDB()
	{
		this->mProto = nullptr;
		this->mMysql = nullptr;
	}

	bool MysqlDB::Awake()
	{
		this->mApp->AddComponent<MysqlDBComponent>();
		return true;
	}

	bool MysqlDB::OnInit()
	{
		BIND_SERVER_RPC_METHOD(MysqlDB::Save);
		BIND_SERVER_RPC_METHOD(MysqlDB::Exec);
		BIND_SERVER_RPC_METHOD(MysqlDB::Query);
		BIND_SERVER_RPC_METHOD(MysqlDB::Index);
		BIND_SERVER_RPC_METHOD(MysqlDB::Update);
		BIND_SERVER_RPC_METHOD(MysqlDB::Delete);
		BIND_SERVER_RPC_METHOD(MysqlDB::Create);
		BIND_SERVER_RPC_METHOD(MysqlDB::Insert);
		BIND_SERVER_RPC_METHOD(MysqlDB::FindPage);
		LOG_CHECK_RET_FALSE(this->mProto = this->GetComponent<ProtoComponent>())
		LOG_CHECK_RET_FALSE(this->mMysql = this->GetComponent<MysqlDBComponent>())
		return true;
	}

    void MysqlDB::OnDestroy()
    {

    }

    int MysqlDB::Create(const db::mysql::create &request)
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

	int MysqlDB::Index(const db::mysql::index& request)
	{
		std::string sql;
		const std::string & tab = request.tab();
		const std::string & field = request.name();
		if(!this->mSqlHelper.CreateIndex(tab, field, request.unique(),sql))
		{
			return XCode::CallArgsError;
		}
		std::unique_ptr<mysql::Request> mysqlRequest = std::make_unique<mysql::Request>(sql);
		std::unique_ptr<mysql::Response> mysqlResponse = this->mMysql->Run(std::move(mysqlRequest));
		return mysqlResponse && mysqlResponse->IsOk() ? XCode::Ok : XCode::Failure;
	}

	int MysqlDB::Insert(const db::mysql::insert& request, db::mysql::response &response)
    {
		json::r::Document document;
		const std::string & tab = request.table();
		const std::string & json = request.document();
		if(!document.Decode(json.c_str(), json.size()))
		{
			return XCode::ParseJsonFailure;
		}
		std::string sql;
		if(!this->mSqlHelper.Insert(tab, document, sql))
		{
			return XCode::Failure;
		}
		std::unique_ptr<mysql::Request> mysqlRequest = std::make_unique<mysql::Request>(sql);
		std::unique_ptr<mysql::Response> mysqlResponse = this->mMysql->Run(std::move(mysqlRequest));
		LOG_ERROR_RETURN_CODE(mysqlResponse != nullptr, XCode::Failure);
		if(!mysqlResponse->IsOk())
		{
			response.set_error(mysqlResponse->GetBuffer());
			return XCode::Failure;
		}
		response.set_count(mysqlResponse->GetOKResponse().mAffectedRows);
		return XCode::Ok;
    }

	int MysqlDB::Save(const db::mysql::save& request, db::mysql::response &response)
    {
		json::r::Document document;
		const std::string & tab = request.table();
		const std::string & json = request.data();
		if(!document.Decode(json.c_str(), json.size()))
		{
			return XCode::ParseJsonFailure;
		}
		std::string sql;
		if (!this->mSqlHelper.Replace(tab, document, sql))
        {
            return XCode::CallArgsError;
        }
		std::unique_ptr<mysql::Request> mysqlRequest = std::make_unique<mysql::Request>(sql);
		std::unique_ptr<mysql::Response> mysqlResponse = this->mMysql->Run(std::move(mysqlRequest));
		LOG_ERROR_RETURN_CODE(mysqlResponse != nullptr, XCode::Failure);
		if(!mysqlResponse->IsOk())
		{
			response.set_error(mysqlResponse->GetBuffer());
			return XCode::Failure;
		}
		response.set_count(mysqlResponse->GetOKResponse().mAffectedRows);
		return XCode::Ok;
    }

	int MysqlDB::Update(const db::mysql::update& request, db::mysql::response & response)
    {
        std::string sql;
		const std::string & table = request.table();
		const std::string & where = request.filter();
		const std::string & update = request.document();
        if (!this->mSqlHelper.Update(table, where, update, request.limit(), sql))
        {
            return XCode::CallArgsError;
        }
		std::unique_ptr<mysql::Request> mysqlRequest = std::make_unique<mysql::Request>(sql);
		std::unique_ptr<mysql::Response> mysqlResponse = this->mMysql->Run(std::move(mysqlRequest));
		LOG_ERROR_RETURN_CODE(mysqlResponse != nullptr, XCode::Failure);

		if(!mysqlResponse->IsOk())
		{
			response.set_error(mysqlResponse->GetBuffer());
			return XCode::Failure;
		}
		response.set_count(mysqlResponse->GetOKResponse().mAffectedRows);
		return XCode::Ok;
    }

	int MysqlDB::Delete(const db::mysql::del& request, db::mysql::response &response)
    {
        std::string sql;
		const std::string & table = request.table();
		const std::string & where = request.filter();
		if (!this->mSqlHelper.Delete(table, where, request.limit(), sql))
        {
            return XCode::CallArgsError;
        }
		std::unique_ptr<mysql::Request> mysqlRequest = std::make_unique<mysql::Request>(sql);
		std::unique_ptr<mysql::Response> mysqlResponse = this->mMysql->Run(std::move(mysqlRequest));
		if(mysqlResponse == nullptr || !mysqlResponse->IsOk())
		{
			return XCode::Failure;
		}
		return XCode::Ok;
    }

	int MysqlDB::Exec(const db::mysql::exec& request, db::mysql::query::response & response)
	{
		const std::string & sql = request.sql();
		std::unique_ptr<mysql::Request> mysqlRequest = std::make_unique<mysql::Request>(sql);
		std::unique_ptr<mysql::Response> mysqlResponse = this->mMysql->Run(std::move(mysqlRequest));
		LOG_ERROR_RETURN_CODE(mysqlResponse != nullptr, XCode::Failure);

		if(!mysqlResponse->IsOk())
		{
			response.set_error(mysqlResponse->GetBuffer());
			return XCode::Failure;
		}
		const mysql::Result & mysqlResult = mysqlResponse->GetResult();
		for(const std::string & result : mysqlResult.contents)
		{
			response.add_jsons(result);
		}
		return XCode::Ok;
	}

	int MysqlDB::Query(const db::mysql::query::request& request, db::mysql::query::response& response)
	{
		const std::string& table = request.table();
		const std::string& where = request.filter();

		LOG_ERROR_CHECK_ARGS(!table.empty());

		std::string sql;
		int limit = request.limit();
		std::vector<std::string> fields;
		if(request.fields_size() > 0)
		{
			fields.reserve(request.fields_size());
			for(int index = 0; index < request.fields_size(); index++)
			{
				fields.emplace_back(request.fields(index));
			}
		}
		if (!this->mSqlHelper.Select(table, where, fields, limit, sql))
		{
			return XCode::CallArgsError;
		}
		std::unique_ptr<mysql::Request> mysqlRequest = std::make_unique<mysql::Request>(sql);
		std::unique_ptr<mysql::Response> mysqlResponse = this->mMysql->Run(std::move(mysqlRequest));
		LOG_ERROR_RETURN_CODE(mysqlResponse != nullptr, XCode::Failure);

		if(!mysqlResponse->IsOk())
		{
			response.set_error(mysqlResponse->GetBuffer());
			return XCode::Failure;
		}
		const mysql::Result & mysqlResult = mysqlResponse->GetResult();
		for(const std::string & result : mysqlResult.contents)
		{
			response.add_jsons(result);
		}
		return XCode::Ok;
	}

	int MysqlDB::FindPage(const db::mysql::query::page& request, db::mysql::query::response& response)
	{
		const std::string& table = request.table();
		const std::string& where = request.filter();

		LOG_ERROR_CHECK_ARGS(!table.empty());

		std::string sql;
		std::vector<std::string> fields;
		if(request.fields_size() > 0)
		{
			fields.reserve(request.fields_size());
			for(int index = 0; index < request.fields_size(); index++)
			{
				fields.emplace_back(request.fields(index));
			}
		}
		if (!this->mSqlHelper.Select(table, where, fields, 0, sql))
		{
			return XCode::CallArgsError;
		}
		int offset = (request.page() - 1) * request.limit();
		sql.append(fmt::format(" LIMIT {}, {}", offset, request.limit()));
		std::unique_ptr<mysql::Request> mysqlRequest = std::make_unique<mysql::Request>(sql);
		std::unique_ptr<mysql::Response> mysqlResponse = this->mMysql->Run(std::move(mysqlRequest));
		LOG_ERROR_RETURN_CODE(mysqlResponse != nullptr, XCode::Failure);

		if(!mysqlResponse->IsOk())
		{
			response.set_error(mysqlResponse->GetBuffer());
			return XCode::Failure;
		}
		const mysql::Result & mysqlResult = mysqlResponse->GetResult();
		for(const std::string & result : mysqlResult.contents)
		{
			response.add_jsons(result);
		}
		return XCode::Ok;
	}
}// namespace Sentry
