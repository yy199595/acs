//
// Created by zmhy0073 on 2022/8/25.
//

#ifndef APP_MYSQLMESSAGE_H
#define APP_MYSQLMESSAGE_H
#include"mysql.h"
#include<memory>
namespace Mysql
{
	class Response
	{

	};
    class ICommand
	{
	 public:
		virtual MYSQL_RES * Invoke(MYSQL *, std::string & error) = 0;
	};

	class SqlCommand : public ICommand
	{
	 public:
		MYSQL_RES * Invoke(MYSQL *, std::string & error) final;
	};
}


#endif //APP_MYSQLMESSAGE_H
