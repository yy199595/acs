//
// Created by 64658 on 2025/6/6.
//

#ifndef APP_PGSQLWRITEPROXY_H
#define APP_PGSQLWRITEPROXY_H
#include "DB/Common/SqlFactory.h"
#include "Rpc/Service/RpcService.h"

namespace acs
{
	class PgsqlWriteProxy final : public RpcService
	{
	public:
		PgsqlWriteProxy();
		~PgsqlWriteProxy() final = default;
	private:
		bool OnInit() final;
	public:
		int Run(const std::string & sql, json::w::Document & response);
		int Inc(const json::r::Document & request, json::w::Document & response);
		int Commit(const json::r::Document & request, json::w::Document & response);
		int Insert(const json::r::Document & request, json::w::Document & response);
		int Execute(const json::r::Document & request, json::w::Document & response);
		int Replace(const json::r::Document & request, json::w::Document & response);
		int Delete(const json::r::Document & request, json::w::Document & response);
		int Update(const json::r::Document & request, json::w::Document & response);
		int SetIndex(const json::r::Document & request, json::w::Document & response);
		int InsertBatch(const json::r::Document & request, json::w::Document & response);
		int UpdateBatch(const json::r::Document & request, json::w::Document & response);
	private:
		sql::Factory mFactory;
		class PgsqlDBComponent * mPgsql;
	};
}



#endif //APP_PGSQLWRITEPROXY_H
