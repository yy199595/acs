#include"MysqlService.h"
#include"Component/MysqlComponent.h"
#include"Scene/ThreadPoolComponent.h"
#include"MysqlClient/MysqlTaskSource.h"
#include"Object/App.h"
#include"Pool/MessagePool.h"
namespace Sentry
{
    bool MysqlService::Awake()
    {
        this->mMysqlComponent = nullptr;
		BIND_RPC_FUNCTION(MysqlService::Add);
		BIND_RPC_FUNCTION(MysqlService::Save);
		BIND_RPC_FUNCTION(MysqlService::Query);
		BIND_RPC_FUNCTION(MysqlService::Delete);
		BIND_RPC_FUNCTION(MysqlService::Invoke);
		BIND_RPC_FUNCTION(MysqlService::Update);
        return true;
    }

    bool MysqlService::LateAwake()
    {
        this->mMysqlComponent = this->GetComponent<MysqlComponent>();
        if(this->mMysqlComponent == nullptr)
        {
            this->mMysqlComponent = this->mEntity->GetOrAddComponent<MysqlComponent>();
            LOG_CHECK_RET_FALSE(this->mMysqlComponent);
            return this->mMysqlComponent->LateAwake();
        }
        return true;
    }

    XCode MysqlService::Add(const s2s::Mysql::Add &request, s2s::Mysql::Response &response)
    {
        LOGIC_THROW_ERROR(request.has_data());
        LOGIC_THROW_ERROR(!request.table().empty());

        std::string sql;
        if(!this->mMysqlComponent->ToSqlCommand(request, sql))
        {
            return XCode::CallArgsError;
        }
        std::shared_ptr<MysqlTaskSource> taskSource(new MysqlTaskSource(this->mMysqlComponent));

        XCode code = taskSource->Await(sql);
        if(code != XCode::Successful)
        {
            response.set_error(taskSource->GetErrorStr());
            return code;
        }
        return XCode::Successful;
    }

    XCode MysqlService::Save(const s2s::Mysql::Save &request, s2s::Mysql::Response &response)
    {
        LOGIC_THROW_ERROR(request.has_data());
        LOGIC_THROW_ERROR(!request.table().empty());

        std::string sql;
        if (!this->mMysqlComponent->ToSqlCommand(request, sql))
        {
            return XCode::CallArgsError;
        }
        std::shared_ptr<MysqlTaskSource> taskSource =
                std::make_shared<MysqlTaskSource>(this->mMysqlComponent);

        XCode code = taskSource->Await(sql);
        if(code != XCode::Successful)
        {
            response.set_error(taskSource->GetErrorStr());
#ifdef __DEBUG__
            LOG_INFO(sql);
            LOG_ERROR(taskSource->GetErrorStr());
#endif
            return code;
        }
        return XCode::Successful;
    }

    XCode MysqlService::Update(const s2s::Mysql::Update &request, s2s::Mysql::Response &response)
	{
		LOGIC_THROW_ERROR(!request.table().empty());
		LOGIC_THROW_ERROR(!request.where_json().empty());

		std::string sql;
		if (!this->mMysqlComponent->ToSqlCommand(request, sql))
		{
			return XCode::CallArgsError;
		}
		std::shared_ptr<MysqlTaskSource> taskSource =
			std::make_shared<MysqlTaskSource>(this->mMysqlComponent);

		XCode code = taskSource->Await(sql);
		if (code != XCode::Successful)
		{
			response.set_error(taskSource->GetErrorStr());
#ifdef __DEBUG__
			LOG_INFO(sql);
			LOG_ERROR(taskSource->GetErrorStr());
#endif
			return code;
		}
		return XCode::Successful;
	}

    XCode MysqlService::Delete(const s2s::Mysql::Delete &request, s2s::Mysql::Response &response)
    {
        LOGIC_THROW_ERROR(!request.table().empty());
        LOGIC_THROW_ERROR(!request.where_json().empty());

        std::string sql;
        if (!this->mMysqlComponent->ToSqlCommand(request, sql)) {
            return XCode::CallArgsError;
        }
        std::shared_ptr<MysqlTaskSource> taskSource =
            std::make_shared<MysqlTaskSource>(this->mMysqlComponent);

        XCode code = taskSource->Await(sql);
        if (code != XCode::Successful)
        {
            response.set_error(taskSource->GetErrorStr());
#ifdef __DEBUG__
            LOG_INFO(sql);
            LOG_ERROR(taskSource->GetErrorStr());
#endif
            return code;
        }
        return XCode::Successful;
    }

    XCode MysqlService::Invoke(const s2s::Mysql::Invoke &request, s2s::Mysql::Response &response)
    {
        LOGIC_THROW_ERROR(!request.sql().empty());
        std::shared_ptr<MysqlTaskSource> taskSource(new MysqlTaskSource(this->mMysqlComponent));

        XCode code = taskSource->Await(request.sql());
        if (code != XCode::Successful)
        {
#ifdef __DEBUG__
            LOG_INFO(request.sql());
            LOG_ERROR(taskSource->GetErrorStr());
#endif
            response.set_error(taskSource->GetErrorStr());
            return code;
        }

        std::string json;
        while (taskSource->GetQueryData(json))
        {
            response.add_json_array(json);
        }
        return XCode::Successful;
    }

    XCode MysqlService::Query(const s2s::Mysql::Query &request, s2s::Mysql::Response &response)
    {
        LOGIC_THROW_ERROR(!request.table().empty());
        LOGIC_THROW_ERROR(!request.where_json().empty());
        std::string sql;
        if (!this->mMysqlComponent->ToSqlCommand(request, sql))
        {
            return XCode::CallArgsError;
        }
        std::shared_ptr<MysqlTaskSource> taskSource
            = std::make_shared<MysqlTaskSource>(this->mMysqlComponent);

        XCode code = taskSource->Await(sql);
        if (code != XCode::Successful)
        {
            response.set_error(taskSource->GetErrorStr());
            return code;
        }

        std::string json;
        while (taskSource->GetQueryData(json))
        {
            response.add_json_array(json);
        }

        return XCode::Successful;
    }
}// namespace Sentry