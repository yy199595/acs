//
// Created by yy on 2023/7/25.
//

#ifndef APP_SQLITECLIENT_H
#define APP_SQLITECLIENT_H
#include<Network/Tcp/Asio.h>
#include"Sqlite/Component/SqliteComponent.h"
namespace Sqlite
{
	class SqliteClient
	{
	public:
		explicit SqliteClient(Asio::Context & io);
	public:

	private:
		Asio::Context & mContext;
	};
}


#endif //APP_SQLITECLIENT_H
