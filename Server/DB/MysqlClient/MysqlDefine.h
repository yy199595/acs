#pragma once
#ifdef _WIN32

#include <WinSock2.h>
#include <windows.h>

#endif

#include<vector>
#include<sstream>
#include"mysql.h"
#include<memory>
#include<unordered_map>
#include<XCode/XCode.h>
#include<Thread/TaskProxy.h>

using namespace std;
using namespace Sentry;
namespace Sentry
{
    class SceneMysqlComponent;

    typedef MYSQL_RES MysqlQueryResult;
    typedef MYSQL SayNoMysqlSocket;
}