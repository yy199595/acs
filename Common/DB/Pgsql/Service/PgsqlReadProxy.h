//
// Created by 64658 on 2025/6/6.
//

#ifndef APP_PGSQLREADPROXY_H
#define APP_PGSQLREADPROXY_H
#include "DB/Common/SqlFactory.h"
#include "Rpc/Service/RpcService.h"
namespace acs
{
	class PgsqlReadProxy : public RpcService
	{
	public:
		PgsqlReadProxy();
		~PgsqlReadProxy() final = default;
	private:
		bool OnInit() final;
	private:
		int Run(const std::string & sql, json::w::Document & response);
		int Func(const json::r::Document & request, rpc::Message & response);
		int Count(const json::r::Document & request, rpc::Message & response);
		int Execute(const json::r::Document & request, rpc::Message & response); //执行预处理sql
		int Distinct(const json::r::Document & request, rpc::Message & response);
	private:
		int Find(const json::r::Document & request, rpc::Message & response);
		int FindOne(const json::r::Document & request, rpc::Message & response);
		int FindPage(const json::r::Document & request, rpc::Message & response);
	private:
		sql::Factory mFactory;
		class PgsqlDBComponent * mPgsql;
	};
}


#endif //APP_PGSQLREADPROXY_H
