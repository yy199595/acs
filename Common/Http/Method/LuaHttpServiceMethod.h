//
// Created by zmhy0073 on 2022/6/6.
//

#ifndef SERVER_LUAHTTPSERVICEMETHOD_H
#define SERVER_LUAHTTPSERVICEMETHOD_H
#include"HttpServiceMethod.h"

struct lua_State;
namespace Sentry
{
    class HttpMethodConfig;
    class LuaHttpServiceMethod : public HttpServiceMethod
    {
    public:
        LuaHttpServiceMethod(const HttpMethodConfig * config, lua_State * lua);
    public:
        bool IsLuaMethod() const { return true; }
        XCode Invoke(const HttpHandlerRequest &request, HttpHandlerResponse &response) final;

    private:
        XCode Call(HttpHandlerResponse & response);
        XCode CallAsync(HttpHandlerResponse & response);
    private:
        lua_State * mLua;
        std::string mData;
        const HttpMethodConfig * mConfig;
    };
}


#endif //SERVER_LUAHTTPSERVICEMETHOD_H
