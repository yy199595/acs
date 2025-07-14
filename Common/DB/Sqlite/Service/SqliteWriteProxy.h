//
// Created by 64658 on 2025/6/6.
//

#ifndef APP_SQLITEWRITEPROXY_H
#define APP_SQLITEWRITEPROXY_H
#include "DB/Common/SqlFactory.h"
#include "Rpc/Service/RpcService.h"


namespace acs
{
	class SqliteWriteProxy final : public RpcService
	{
	public:
		SqliteWriteProxy();
		~SqliteWriteProxy() final = default;
	private:
		bool OnInit() final;
	public:
		int Run(const std::string & sql, json::w::Document & response);
		int Inc(const json::r::Document & request, json::w::Document & response);
		int Commit(const json::r::Document & request, json::w::Document & response);
		int Replace(const json::r::Document & request, json::w::Document & response);
		int Delete(const json::r::Document & request, json::w::Document & response);
		int Update(const json::r::Document & request, json::w::Document & response);
		int SetIndex(const json::r::Document & request, json::w::Document & response);
		int InsertOne(const json::r::Document & request, json::w::Document & response);
		int InsertBatch(const json::r::Document & request, json::w::Document & response);
	private:
		sql::Factory mFactory;
		class SqliteComponent * mSqlite;
	};
}



#endif //APP_SQLITEWRITEPROXY_H
