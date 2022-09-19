//
// Created by zmhy0073 on 2022/8/25.
//

#include"MysqlMessage.h"
#include"errmsg.h"

namespace Mysql
{
    Response::Response(MYSQL_RES *result)
    {
        this->mResult = result;
    }

    Response::Response(const std::string &error)
        : mError(error)
    {
        this->mResult = nullptr;
    }

    Response::~Response()
    {
        if(this->mResult != nullptr)
        {
            mysql_free_result(this->mResult);
        }
    }
}

namespace Mysql
{
    MYSQL_RES *PingCommand::Invoke(MYSQL * sock, std::string &error)
    {
        switch(mysql_ping(sock))
        {
            case 0:
                break;
            case CR_UNKNOWN_ERROR:
                error = "CR_UNKNOWN_ERROR";
                break;
            case CR_SERVER_GONE_ERROR:
                error = "CR_SERVER_GONE_ERROR";
                break;
            case CR_COMMANDS_OUT_OF_SYNC:
                error = "CR_COMMANDS_OUT_OF_SYNC";
                break;
        }
        return nullptr;
    }

	MYSQL_RES *SqlCommand::Invoke(MYSQL*, std::string & error)
	{
		return nullptr;
	}

    MYSQL_RES *QueryCommand::Invoke(MYSQL *, std::string &error)
    {
        return nullptr;
    }
}

namespace Mysql
{
    MYSQL_RES *InitCommand::Invoke(MYSQL * sock, std::string &error)
    {
        MYSQL_RES * res = mysql_list_dbs(sock, NULL);
        if(res == nullptr)
        {
            error.append(mysql_error(sock));
            return nullptr;
        }
        while(st_mysql_field * field = mysql_fetch_field(res))
        {
            printf(field->name);
        }
        return res;
    }
}