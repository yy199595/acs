//
// Created by yy on 2023/7/25.
//

#ifndef APP_SQLITEMESSAGE_H
#define APP_SQLITEMESSAGE_H
#include<string>
#include<vector>
namespace Sqlite
{
	class Request
	{
	public:
		Request(std::string sql, int id)
			: mSql(std::move(sql)), mRpcId(id) { }
	private:
		int mRpcId;
		std::string mSql;
	};

	class Response
	{
	private:
		std::string mError;
		std::vector<std::string> mJsons;
	};
}


#endif //APP_SQLITEMESSAGE_H
