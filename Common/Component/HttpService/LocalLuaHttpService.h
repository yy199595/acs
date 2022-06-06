//
// Created by zmhy0073 on 2022/6/6.
//

#ifndef SERVER_LOCALLUAHTTPSERVICE_H
#define SERVER_LOCALLUAHTTPSERVICE_H
#include"LocalHttpService.h"
namespace Sentry
{
    class LocalLuaHttpService : public LocalHttpService
    {
    public:
        LocalLuaHttpService() = default;
        ~LocalLuaHttpService() = default;
    private:
        bool LateAwake() final;
        bool OnStartService(HttpServiceRegister &serviceRegister) final;
    };
}


#endif //SERVER_LOCALLUAHTTPSERVICE_H
