#pragma once
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <Windows.h>

#endif

#include<vector>
#include<sstream>
#include"mysql.h"
#include<memory>
#include<unordered_map>
#include"XCode/XCode.h"
#include"Thread/IThreadTask.h"

using namespace std;
using namespace Sentry;
namespace Sentry
{
    class MysqlHelper;

    typedef MYSQL_RES MysqlQueryResult;
    typedef MYSQL MysqlSocket;
}

namespace Sentry
{

	class MysqlConfig
	{
	public:
		std::string mIp;
		unsigned int mPort;
		std::string mUser;
		std::string mPassword;
	};

	class MysqlAsyncTask
	{
	 public:
		virtual XCode Await() = 0;
		virtual void Run(MysqlSocket * mysql) = 0;
		virtual bool Init() { return true;}
	};
}