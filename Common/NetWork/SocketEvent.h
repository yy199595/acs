#pragma once

#include<string>
#include<memory>
#include"PacketMapper.h"

using namespace std;
namespace Sentry
{
    enum SessionType
    {
        SessionNone,
        SessionClient, //外部连接进来的
        SessionNode        //连接到外部的
    };
}