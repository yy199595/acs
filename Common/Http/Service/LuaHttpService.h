//
// Created by zmhy0073 on 2022/6/6.
//

#ifndef SERVER_LOCALLUAHTTPSERVICE_H
#define SERVER_LOCALLUAHTTPSERVICE_H
#include"LocalHttpService.h"
namespace Sentry
{
    class LuaHttpService : public LocalHttpService
    {
    public:
        LuaHttpService() = default;
        ~LuaHttpService() = default;
    private:
        bool LateAwake() final;
        bool OnCloseService() final;
        bool OnStartService(HttpServiceRegister &serviceRegister) final;

    private:
        struct lua_State * mLuaEnv;
    };
}


#endif //SERVER_LOCALLUAHTTPSERVICE_H
