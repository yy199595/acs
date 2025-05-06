

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
		BIND_RPC_METHOD(MysqlDB::Save);
		BIND_RPC_METHOD(MysqlDB::Exec);
		BIND_RPC_METHOD(MysqlDB::Query);
		BIND_RPC_METHOD(MysqlDB::Index);
		BIND_RPC_METHOD(MysqlDB::Update);
		BIND_RPC_METHOD(MysqlDB::Delete);
		BIND_RPC_METHOD(MysqlDB::Create);
		BIND_RPC_METHOD(MysqlDB::Insert);
		BIND_RPC_METHOD(MysqlDB::FindPage);
		LOG_CHECK_RET_FALSE(this->mProto = this->GetComponent<ProtoComponent>())
		LOG_CHECK_RET_FALSE(this->mMysql = this->GetComponent<MysqlDBComponent>())
		return true;
	}

    int MysqlDB::Create(const db::sql::create &request, db::sql::response &response)
	{
		std::unique_ptr<Message> message;
		if (!this->mProto->New(request.table(), message))
		{
			return XCode::CallArgsError;
		}
		std::vector<std::string> keys;
		const Descriptor* descriptor = message->GetDescriptor();
		for (const std::string& key: request.keys())
		{
			const FieldDescriptor* fieldDescriptor = descriptor->FindFieldByName(key);
			if (fieldDescriptor == nullptr)
			{
				return XCode::CallArgsError;
			}
			switch (fieldDescriptor->type())
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
		std::string sql;
		if (!this->mSqlHelper.Create(request.table(), *message, keys, sql))
		{
			return XCode::CallArgsError;
		}
		std::unique_ptr<mysql::Response> mysqlResponse = this->mMysql->Run(sql);
		LOG_ERROR_RETURN_CODE(mysqlResponse != nullptr, XCode::Failure);
		if (!mysqlResponse->IsOk())
		{
			response.set_error(mysqlResponse->GetBuffer());
			return XCode::Failure;
		}
		response.set_count(mysqlResponse->GetOKResponse().mAffectedRows);
		return XCode::Ok;
	}

	int MysqlDB::Index(const db::sql::index& request)
	{
		std::string sql;
		if(!this->mSqlHelper.CreateIndex(request, sql))
		{
			return XCode::CallArgsError;
		}
		std::unique_ptr<mysql::Response> mysqlResponse = this->mMysql->Run(sql);
		return mysqlResponse && mysqlResponse->IsOk() ? XCode::Ok : XCode::Failure;
	}

	int MysqlDB::Insert(const db::sql::insert& request, db::sql::response &response)
    {
		std::string sql;
		if(!this->mSqlHelper.Insert(request, sql))
		{
			return XCode::CallArgsError;
		}
		std::unique_ptr<mysql::Response> mysqlResponse = this->mMysql->Run(sql);
		LOG_ERROR_RETURN_CODE(mysqlResponse != nullptr, XCode::Failure);
		if(!mysqlResponse->IsOk())
		{
			response.set_error(mysqlResponse->GetBuffer());
			return XCode::Failure;
		}
		response.set_count(mysqlResponse->GetOKResponse().mAffectedRows);
		return XCode::Ok;
    }

	int MysqlDB::Save(const db::sql::save& request, db::sql::response &response)
    {
		std::string sql;
		if (!this->mSqlHelper.Replace(request, sql))
        {
            return XCode::CallArgsError;
        }
		std::unique_ptr<mysql::Response> mysqlResponse = this->mMysql->Run(sql);
		LOG_ERROR_RETURN_CODE(mysqlResponse != nullptr, XCode::Failure);
		if(!mysqlResponse->IsOk())
		{
			response.set_error(mysqlResponse->GetBuffer());
			return XCode::Failure;
		}
		response.set_count(mysqlResponse->GetOKResponse().mAffectedRows);
		return XCode::Ok;
    }

	int MysqlDB::Update(const db::sql::update& request, db::sql::response & response)
    {
        std::string sql;
        if (!this->mSqlHelper.Update(request, sql))
        {
            return XCode::CallArgsError;
        }
		std::unique_ptr<mysql::Response> mysqlResponse = this->mMysql->Run(sql);
		LOG_ERROR_RETURN_CODE(mysqlResponse != nullptr, XCode::Failure);

		if(!mysqlResponse->IsOk())
		{
			response.set_error(mysqlResponse->GetBuffer());
			return XCode::Failure;
		}
		response.set_count(mysqlResponse->GetOKResponse().mAffectedRows);
		return XCode::Ok;
    }

	int MysqlDB::Delete(const db::sql::del& request, db::sql::response &response)
    {
        std::string sql;
		if (!this->mSqlHelper.Delete(request, sql))
        {
            return XCode::CallArgsError;
        }
		std::unique_ptr<mysql::Response> mysqlResponse = this->mMysql->Run(sql);
		if(mysqlResponse == nullptr || !mysqlResponse->IsOk())
		{
			return XCode::Failure;
		}
		return XCode::Ok;
    }

	int MysqlDB::Exec(const db::sql::exec& request, db::sql::query::response & response)
	{
		const std::string & sql = request.sql();
		std::unique_ptr<mysql::Response> mysqlResponse = this->mMysql->Run(sql);
		LOG_ERROR_RETURN_CODE(mysqlResponse != nullptr, XCode::Failure);

		if(!mysqlResponse->IsOk())
		{
			response.set_error(mysqlResponse->GetBuffer());
			return XCode::Failure;
		}
		const mysql::Result & mysqlResult = mysqlResponse->GetResult();
		for(const std::string & result : mysqlResult.contents)
		{
			response.add_documents(result);
		}
		return XCode::Ok;
	}

	int MysqlDB::Query(const db::sql::query::request& request, db::sql::query::response& response)
	{
		std::string sql;
		if (!this->mSqlHelper.Select(request, sql))
		{
			return XCode::CallArgsError;
		}
		std::unique_ptr<mysql::Response> mysqlResponse = this->mMysql->Run(sql);
		LOG_ERROR_RETURN_CODE(mysqlResponse != nullptr, XCode::Failure);

		if(!mysqlResponse->IsOk())
		{
			response.set_error(mysqlResponse->GetBuffer());
			return XCode::Failure;
		}
		const mysql::Result & mysqlResult = mysqlResponse->GetResult();
		for(const std::string & result : mysqlResult.contents)
		{
			response.add_documents(result);
		}
		return XCode::Ok;
	}

	int MysqlDB::FindPage(const db::sql::query::page& request, db::sql::query::response& response)
	{
		std::string sql;
		if (!this->mSqlHelper.FindPage(request, sql))
		{
			return XCode::CallArgsError;
		}
		std::unique_ptr<mysql::Response> mysqlResponse = this->mMysql->Run(sql);
		LOG_ERROR_RETURN_CODE(mysqlResponse != nullptr, XCode::Failure);

		if(!mysqlResponse->IsOk())
		{
			response.set_error(mysqlResponse->GetBuffer());
			return XCode::Failure;
		}
		const mysql::Result & mysqlResult = mysqlResponse->GetResult();
		for(const std::string & result : mysqlResult.contents)
		{
			response.add_documents(result);
		}
		return XCode::Ok;
	}
}// namespace Sentry
