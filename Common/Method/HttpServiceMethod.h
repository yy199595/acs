//
// Created by zmhy0073 on 2021/11/2.
//

#ifndef GAMEKEEPER_HTTPSERVICEMETHOD_H
#define GAMEKEEPER_HTTPSERVICEMETHOD_H
#include <Http/Http.h>
#include <Util/JsonHelper.h>
#include <Util/TimeHelper.h>
namespace GameKeeper
{
    template<typename T>
    using HttpServiceJsonMethodRequestType = XCode(T::*)(RapidJsonWriter & response);

    template<typename T>
    using HttpServiceJsonMethodType = XCode(T::*)(const RapidJsonReader & request, RapidJsonWriter & response);

    class HttpServiceMethod
    {
    public:
        virtual HttpStatus Invoke(const std::string & content) = 0;
    };

}
#endif //GAMEKEEPER_HTTPSERVICEMETHOD_H
