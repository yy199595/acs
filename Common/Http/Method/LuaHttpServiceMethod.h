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
        LuaHttpServiceMethod(const HttpMethodConfig * config);
    public:
        bool IsLuaMethod() const { return true; }
        XCode Invoke(const Http::Request &request, Http::Response &response) final;

    private:
        XCode Call(Http::Response & response);
        XCode CallAsync(Http::Response & response);
    private:
        lua_State * mLua;
        std::string mData;
        const HttpMethodConfig * mConfig;
        class LuaScriptComponent* mLuaComponent;
    };
}


#endif //SERVER_LUAHTTPSERVICEMETHOD_H
