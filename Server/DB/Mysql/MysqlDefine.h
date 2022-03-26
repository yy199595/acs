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
#include"Thread/TaskProxy.h"

using namespace std;
using namespace Sentry;
namespace Sentry
{
    class MysqlComponent;

    typedef MYSQL_RES MysqlQueryResult;
    typedef MYSQL MysqlClient;
}