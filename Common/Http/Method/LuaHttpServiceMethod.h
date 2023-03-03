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
        int Invoke(const Http::Request &request, Http::Response &response) final;

    private:
        int Call(Http::Response & response);
        int CallAsync(Http::Response & response);
    private:           
        const HttpMethodConfig * mConfig;
        class LuaScriptComponent* mLuaComponent;
    };
}


#endif //SERVER_LUAHTTPSERVICEMETHOD_H
