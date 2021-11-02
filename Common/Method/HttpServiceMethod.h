//
// Created by zmhy0073 on 2021/11/2.
//

#ifndef GAMEKEEPER_HTTPSERVICEMETHOD_H
#define GAMEKEEPER_HTTPSERVICEMETHOD_H
#include <Network/Http/Http.h>
#include <Network/Http/HttpRemoteSession.h.h>
namespace GameKeeper
{
    template<typename T>
    using ServiceMethodType1 = HttpStatus(T::*)(HttpRemoteSession *);

}
#endif //GAMEKEEPER_HTTPSERVICEMETHOD_H
